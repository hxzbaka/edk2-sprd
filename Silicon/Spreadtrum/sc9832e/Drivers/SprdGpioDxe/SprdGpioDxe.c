/** @file
  SprdGpioDxe.c

  GPIO / EIC (Debounce, Latch, Async, Sync) / PMIC driver
  for Spreadtrum UMS512 (SharkL5 Pro)

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Protocol/SprdGpioProtocol.h>
#include "SprdAdi.h"

/* ---------- Common GPIO ---------- */
#define AP_GPIO_BASE          FixedPcdGet32(PcdApGpioBaseAddress)
#define GPIO_GROUP_NR         16
#define GPIO_GROUP_OFFSET     0x80

#define REG_DATA              0x00
#define REG_DMSK              0x04
#define REG_DIR               0x08
#define REG_INEN              0x28

/* ---------- SC9832E/SharkLE AP EIC (3 banks, 8 pins each) ---------- */
#define EIC_DEB_BANK_NR       3
#define EIC_DEB_PINS_PER_BANK 8

STATIC CONST UINT32 EicDebBase[EIC_DEB_BANK_NR] = {
  0x40210000, 0x40370000, 0x402C0000
};
#define EIC_DEB_DATA          0x00
#define EIC_DEB_DMSK          0x04

/* ---------- Latch EIC ---------- */
STATIC CONST UINT32 EicLatchBase[EIC_DEB_BANK_NR] = {
  0x40210080, 0x40370080, 0x402C0080
};
/* No data register for Latch, only interrupt registers.
 * Not supporting Get(). */

/* ---------- Async EIC ---------- */
STATIC CONST UINT32 EicAsyncBase[EIC_DEB_BANK_NR] = {
  0x402100A0, 0x403700A0, 0x402C00A0
};
#define EIC_ASYNC_DATA        0x1C

/* ---------- Sync EIC ---------- */
STATIC CONST UINT32 EicSyncBase[EIC_DEB_BANK_NR] = {
  0x402100C0, 0x403700C0, 0x402C00C0
};
#define EIC_SYNC_DATA         0x1C

/* ---------- PMIC EIC ---------- */
#define PMIC_EIC_BASE         0x280
#define PMIC_EIC_PINS         16

/* ==============================
 *       GPIO  helpers
 * ============================== */
STATIC UINT32 ApGpioAddr(UINT32 Gpio, UINT32 Reg) {
  return AP_GPIO_BASE + (Gpio / GPIO_GROUP_NR) * GPIO_GROUP_OFFSET + Reg;
}

STATIC EFI_STATUS ApGpioRequest(UINT32 Gpio) {
  MmioOr32(ApGpioAddr(Gpio, REG_DMSK), 1U << (Gpio % GPIO_GROUP_NR));
  return EFI_SUCCESS;
}

STATIC BOOLEAN ApGpioGet(UINT32 Gpio) {
  return (MmioRead32(ApGpioAddr(Gpio, REG_DATA)) >> (Gpio % GPIO_GROUP_NR)) & 1;
}

STATIC EFI_STATUS ApGpioSet(UINT32 Gpio, BOOLEAN Level) {
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  if (Level)
    MmioOr32(ApGpioAddr(Gpio, REG_DATA), Bit);
  else
    MmioAnd32(ApGpioAddr(Gpio, REG_DATA), ~Bit);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS ApGpioDirectionInput(UINT32 Gpio) {
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  ApGpioRequest(Gpio);
  MmioAnd32(ApGpioAddr(Gpio, REG_DIR), ~Bit);
  MmioOr32(ApGpioAddr(Gpio, REG_INEN), Bit);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS ApGpioDirectionOutput(UINT32 Gpio, BOOLEAN Level) {
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  ApGpioRequest(Gpio);
  ApGpioSet(Gpio, Level);
  MmioAnd32(ApGpioAddr(Gpio, REG_INEN), ~Bit);
  MmioOr32(ApGpioAddr(Gpio, REG_DIR), Bit);
  return EFI_SUCCESS;
}

/* ==============================
 *     Debounce EIC helpers
 * ============================== */
STATIC EFI_STATUS EicDebounceRequest(UINT32 EicNo) {
  UINT32 Bank = EicNo / EIC_DEB_PINS_PER_BANK;
  UINT32 Bit  = 1U << (EicNo % EIC_DEB_PINS_PER_BANK);
  if (Bank >= EIC_DEB_BANK_NR) return EFI_INVALID_PARAMETER;
  MmioOr32(EicDebBase[Bank] + EIC_DEB_DMSK, Bit);
  return EFI_SUCCESS;
}

STATIC BOOLEAN EicDebounceGet(UINT32 EicNo) {
  UINT32 Bank = EicNo / EIC_DEB_PINS_PER_BANK;
  if (Bank >= EIC_DEB_BANK_NR) return FALSE;
  return (MmioRead32(EicDebBase[Bank] + EIC_DEB_DATA) >> (EicNo % EIC_DEB_PINS_PER_BANK)) & 1;
}

STATIC EFI_STATUS EicDebounceDirectionInput(UINT32 EicNo) {
  return EicDebounceRequest(EicNo);
}

/* ==============================
 *     Latch EIC helpers
 * ============================== */
STATIC EFI_STATUS EicLatchDirectionInput(UINT32 EicNo) {
  /* Latch EIC has no data read, but we can still enable the pin
   * by setting the appropriate INTEN or DMSK? The kernel driver
   * does not provide get/set. For direction input, we do nothing. */
  return EFI_SUCCESS;
}

/* ==============================
 *     Async EIC helpers
 * ============================== */
STATIC BOOLEAN EicAsyncGet(UINT32 EicNo) {
  UINT32 Bank = EicNo / EIC_DEB_PINS_PER_BANK;
  if (Bank >= EIC_DEB_BANK_NR) return FALSE;
  return (MmioRead32(EicAsyncBase[Bank] + EIC_ASYNC_DATA) >> (EicNo % EIC_DEB_PINS_PER_BANK)) & 1;
}

STATIC EFI_STATUS EicAsyncDirectionInput(UINT32 EicNo) {
  /* Async EIC reads work without explicit request. */
  return EFI_SUCCESS;
}

/* ==============================
 *     Sync EIC helpers
 * ============================== */
STATIC BOOLEAN EicSyncGet(UINT32 EicNo) {
  UINT32 Bank = EicNo / EIC_DEB_PINS_PER_BANK;
  if (Bank >= EIC_DEB_BANK_NR) return FALSE;
  return (MmioRead32(EicSyncBase[Bank] + EIC_SYNC_DATA) >> (EicNo % EIC_DEB_PINS_PER_BANK)) & 1;
}

STATIC EFI_STATUS EicSyncDirectionInput(UINT32 EicNo) {
  return EFI_SUCCESS;
}

/* ==============================
 *     PMIC EIC helpers
 * ============================== */
STATIC EFI_STATUS PmicEicRequest(UINT32 EicNo) {
  if (EicNo >= PMIC_EIC_PINS) return EFI_INVALID_PARAMETER;
  UINT16 Mask = AdiRead(PMIC_EIC_BASE + REG_DMSK);
  Mask |= (UINT16)(1U << EicNo);
  AdiWrite(PMIC_EIC_BASE + REG_DMSK, Mask);
  return EFI_SUCCESS;
}

STATIC BOOLEAN PmicEicGet(UINT32 EicNo) {
  if (EicNo >= PMIC_EIC_PINS) return FALSE;
  PmicEicRequest(EicNo);
  UINT16 Data = AdiRead(PMIC_EIC_BASE + REG_DATA);
  return (Data >> EicNo) & 1;
}

STATIC EFI_STATUS PmicEicDirectionInput(UINT32 EicNo) {
  return PmicEicRequest(EicNo);
}

/* ==============================
 *   Unified protocol interface
 * ============================== */
STATIC BOOLEAN EFIAPI SprdGet(UINT32 Gpio) {
  if (Gpio < 0x100) {
    return ApGpioGet(Gpio);
  }
  if (Gpio >= 0x120 && Gpio < 0x140) {
    return EicDebounceGet(Gpio - 0x120);
  }
  if (Gpio >= 0x140 && Gpio < 0x150) {
    return PmicEicGet(Gpio - 0x140);
  }
  if (Gpio >= 0x150 && Gpio < 0x170) {
    /* Latch EIC: no data read */
    return FALSE;
  }
  if (Gpio >= 0x170 && Gpio < 0x190) {
    return EicAsyncGet(Gpio - 0x170);
  }
  if (Gpio >= 0x190 && Gpio < 0x1B0) {
    return EicSyncGet(Gpio - 0x190);
  }
  return FALSE;
}

STATIC EFI_STATUS EFIAPI SprdDirectionInput(UINT32 Gpio) {
  if (Gpio < 0x100) {
    return ApGpioDirectionInput(Gpio);
  }
  if (Gpio >= 0x120 && Gpio < 0x140) {
    return EicDebounceDirectionInput(Gpio - 0x120);
  }
  if (Gpio >= 0x140 && Gpio < 0x150) {
    return PmicEicDirectionInput(Gpio - 0x140);
  }
  if (Gpio >= 0x150 && Gpio < 0x170) {
    return EicLatchDirectionInput(Gpio - 0x150);
  }
  if (Gpio >= 0x170 && Gpio < 0x190) {
    return EicAsyncDirectionInput(Gpio - 0x170);
  }
  if (Gpio >= 0x190 && Gpio < 0x1B0) {
    return EicSyncDirectionInput(Gpio - 0x190);
  }
  return EFI_UNSUPPORTED;
}

STATIC EFI_STATUS EFIAPI SprdDirectionOutput(UINT32 Gpio, BOOLEAN Value) {
  if (Gpio < 0x100) {
    return ApGpioDirectionOutput(Gpio, Value);
  }
  return EFI_UNSUPPORTED;
}

STATIC EFI_STATUS EFIAPI SprdSet(UINT32 Gpio, BOOLEAN Value) {
  if (Gpio < 0x100) {
    return ApGpioSet(Gpio, Value);
  }
  return EFI_UNSUPPORTED;
}

STATIC SPRD_GPIO_PROTOCOL mSprdGpio = {
  SPRD_GPIO_PROTOCOL_REVISION,
  SprdDirectionInput,
  SprdDirectionOutput,
  SprdGet,
  SprdSet
};

EFI_STATUS EFIAPI SprdGpioEntryPoint(
  IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  return gBS->InstallProtocolInterface(
                &Handle, &gSprdGpioProtocolGuid,
                EFI_NATIVE_INTERFACE, &mSprdGpio);
}