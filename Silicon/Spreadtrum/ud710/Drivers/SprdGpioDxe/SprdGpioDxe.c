/** @file
  SprdGpioDxe.c

  Gpio/Adi/EIC/PMIC driver for Spreadtrum UD710 (roc1)

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#include "SprdAdi.h"
#include <Protocol/SprdGpioProtocol.h>

/* AP GPIO */
#define AP_GPIO_BASE        0x32070000
#define GPIO_GROUP_NR       16
#define REG_DATA            0x00
#define REG_DMSK            0x04
#define REG_DIR             0x08
#define REG_INEN            0x28
#define GPIO_GROUP_OFFSET   0x80

/* AP EIC */
#define EIC_PER_BANK_NR     8
#define EIC_BANK_NUM        4           /* sharkl5: 4 banks */
#define EIC_DBNC_DATA       0x0
#define EIC_DBNC_DMSK       0x4

/* PMIC EIC */
#define PMIC_EIC_BASE       0x280

/* Virtual GPIO ranges */
#define GPIO_AP_START       0x000
#define GPIO_AP_END         0x0FF
#define GPIO_EIC_START      0x120
#define GPIO_EIC_END        0x13F
#define GPIO_PMIC_EIC_START 0x140
#define GPIO_PMIC_EIC_END   0x14F

/*
 * Debounce EIC bank base addresses (from DTS gpio@32000000).
 * Order: bank0, bank1, bank2, bank3
 */
STATIC CONST UINT32 mEicBankBase[EIC_BANK_NUM] = {
  0x32020000,
  0x32030000,
  0x32230000,
  0x32270000
};

/* ---------- AP GPIO ---------- */
STATIC
UINT32
ApGpioAddr (
  UINT32 Gpio,
  UINT32 Reg
  )
{
  return AP_GPIO_BASE + (Gpio / GPIO_GROUP_NR) * GPIO_GROUP_OFFSET + Reg;
}

STATIC
EFI_STATUS
ApGpioRequest (
  UINT32 Gpio
  )
{
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  MmioOr32 (ApGpioAddr (Gpio, REG_DMSK), Bit);
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
ApGpioGet (
  UINT32 Gpio
  )
{
  ApGpioRequest (Gpio);
  UINT32 Val = MmioRead32 (ApGpioAddr (Gpio, REG_DATA));
  return (BOOLEAN)((Val >> (Gpio % GPIO_GROUP_NR)) & 1);
}

STATIC
EFI_STATUS
ApGpioSet (
  UINT32 Gpio,
  BOOLEAN Level
  )
{
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  if (Level) {
    MmioOr32 (ApGpioAddr (Gpio, REG_DATA), Bit);
  } else {
    MmioAnd32 (ApGpioAddr (Gpio, REG_DATA), ~Bit);
  }
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ApGpioDirectionInput (
  UINT32 Gpio
  )
{
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  ApGpioRequest (Gpio);
  MmioAnd32 (ApGpioAddr (Gpio, REG_DIR), ~Bit);
  MmioOr32 (ApGpioAddr (Gpio, REG_INEN), Bit);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ApGpioDirectionOutput (
  UINT32 Gpio,
  BOOLEAN Level
  )
{
  UINT32 Bit = 1U << (Gpio % GPIO_GROUP_NR);
  ApGpioRequest (Gpio);
  ApGpioSet (Gpio, Level);
  MmioAnd32 (ApGpioAddr (Gpio, REG_INEN), ~Bit);
  MmioOr32 (ApGpioAddr (Gpio, REG_DIR), Bit);
  return EFI_SUCCESS;
}

/* ---------- AP EIC (Debounce) ---------- */
STATIC
UINT32
ApEicAddr (
  UINT32 EicId
  )
{
  UINT32 Bank = EicId / EIC_PER_BANK_NR;
  if (Bank >= EIC_BANK_NUM) {
    return 0;
  }
  return mEicBankBase[Bank];
}

STATIC
EFI_STATUS
ApEicRequest (
  UINT32 EicId
  )
{
  UINT32 Base = ApEicAddr (EicId);
  if (Base == 0) {
    return EFI_INVALID_PARAMETER;
  }
  UINT32 Bit = 1U << (EicId % EIC_PER_BANK_NR);
  MmioOr32 (Base + EIC_DBNC_DMSK, Bit);
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
ApEicGet (
  UINT32 EicId
  )
{
  UINT32 Base = ApEicAddr (EicId);
  if (Base == 0) {
    return FALSE;
  }
  ApEicRequest (EicId);
  UINT32 Val = MmioRead32 (Base + EIC_DBNC_DATA);
  return (BOOLEAN)((Val >> (EicId % EIC_PER_BANK_NR)) & 1);
}

STATIC
EFI_STATUS
ApEicDirectionInput (
  UINT32 EicId
  )
{
  return ApEicRequest (EicId);
}

/* ---------- PMIC EIC ---------- */
STATIC
EFI_STATUS
PmicEicRequest (
  UINT32 Eic
  )
{
  UINT16 Mask;

  if (Eic >= 16) {
    return EFI_INVALID_PARAMETER;
  }
  Mask = AdiRead (PMIC_EIC_BASE + REG_DMSK);
  Mask |= (UINT16)(1U << Eic);
  AdiWrite (PMIC_EIC_BASE + REG_DMSK, Mask);
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
PmicEicGet (
  UINT32 Eic
  )
{
  if (Eic >= 16) {
    return FALSE;
  }
  PmicEicRequest (Eic);
  UINT16 Data = AdiRead (PMIC_EIC_BASE + REG_DATA);
  return (BOOLEAN)((Data >> Eic) & 1);
}

STATIC
EFI_STATUS
PmicEicDirectionInput (
  UINT32 Eic
  )
{
  return PmicEicRequest (Eic);
}

/* ---------- Protocol Interfaces ---------- */
STATIC
EFI_STATUS
EFIAPI
SprdSet (
  UINT32 Gpio,
  BOOLEAN Value
  )
{
  if (Gpio >= GPIO_AP_START && Gpio <= GPIO_AP_END) {
    return ApGpioSet (Gpio, Value);
  }
  return EFI_UNSUPPORTED;
}

STATIC
BOOLEAN
EFIAPI
SprdGet (
  UINT32 Gpio
  )
{
  if (Gpio >= GPIO_AP_START && Gpio <= GPIO_AP_END) {
    return ApGpioGet (Gpio);
  }
  if (Gpio >= GPIO_EIC_START && Gpio <= GPIO_EIC_END) {
    return ApEicGet (Gpio - GPIO_EIC_START);
  }
  if (Gpio >= GPIO_PMIC_EIC_START && Gpio <= GPIO_PMIC_EIC_END) {
    return PmicEicGet (Gpio - GPIO_PMIC_EIC_START);
  }
  return FALSE;
}

STATIC
EFI_STATUS
EFIAPI
SprdDirectionInput (
  UINT32 Gpio
  )
{
  if (Gpio >= GPIO_AP_START && Gpio <= GPIO_AP_END) {
    return ApGpioDirectionInput (Gpio);
  }
  if (Gpio >= GPIO_EIC_START && Gpio <= GPIO_EIC_END) {
    return ApEicDirectionInput (Gpio - GPIO_EIC_START);
  }
  if (Gpio >= GPIO_PMIC_EIC_START && Gpio <= GPIO_PMIC_EIC_END) {
    return PmicEicDirectionInput (Gpio - GPIO_PMIC_EIC_START);
  }
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
SprdDirectionOutput (
  UINT32 Gpio,
  BOOLEAN Value
  )
{
  if (Gpio >= GPIO_AP_START && Gpio <= GPIO_AP_END) {
    return ApGpioDirectionOutput (Gpio, Value);
  }
  return EFI_INVALID_PARAMETER;
}

STATIC SPRD_GPIO_PROTOCOL mSprdGpio = {
  SPRD_GPIO_PROTOCOL_REVISION,
  SprdDirectionInput,
  SprdDirectionOutput,
  SprdGet,
  SprdSet
};

EFI_STATUS
EFIAPI
SprdGpioEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gSprdGpioProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSprdGpio
                  );
  DEBUG ((DEBUG_ERROR, "[SprdGpioDxe] Loaded (UD710)\n"));
  return Status;
}