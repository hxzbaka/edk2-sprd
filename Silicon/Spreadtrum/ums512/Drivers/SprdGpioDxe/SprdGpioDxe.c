/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 hxzbaka
 *
 * Gpio/Adi/PMIC_EIC driver for Spreadtrum UMS512 (SharkL5Pro)
 */

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>

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
#define PMIC_EIC_ALL_MASK   0xFFFFU
#define PMIC_EIC_CTRL0      0x40U
#define PMIC_EIC_CTRL4      (PMIC_EIC_CTRL0 + (4U * 4U))

#define AON_APB_BASE        0x327D0000U
#define AON_APB_EB2         0x0008U
#define AON_APB_RTC_EB0     0x0018U
#define AON_EIC_EB          BIT10
#define AON_ADI_EB          BIT9
#define AON_EIC_RTC_EB      BIT5
#define AON_EIC_RTCDV5_EB   BIT6

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
  MicroSecondDelay (20);
  UINT16 Data = AdiRead (PMIC_EIC_BASE + REG_DATA);
  MicroSecondDelay (20);
  Data = AdiRead (PMIC_EIC_BASE + REG_DATA);
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

STATIC
VOID
SprdGpioInit (
  VOID
  )
{
  UINT32 Index;

  /* eic init */
  MmioOr32 (AON_APB_BASE + AON_APB_EB2, AON_ADI_EB | AON_EIC_EB);
  MmioOr32 (AON_APB_BASE + AON_APB_RTC_EB0,
            AON_EIC_RTC_EB | AON_EIC_RTCDV5_EB);

  /* AP EIC banks: DATA/DMSK are 8-bit banks on SharkL5Pro. */
  for (Index = 0; Index < EIC_BANK_NUM; Index++) {
    MmioOr32 (mEicBankBase[Index] + EIC_DBNC_DMSK, 0xFFU);
  }

  /* ADIE EIC DATA mask: preserve the value, then expose all channels. */
  {
    UINT16 Mask = AdiRead (PMIC_EIC_BASE + REG_DMSK);
    UINT16 NewMask = (UINT16)(Mask | PMIC_EIC_ALL_MASK);
    if (NewMask != Mask) {
      AdiWrite (PMIC_EIC_BASE + REG_DMSK, NewMask);
    }

    /* Match the 2 ms debounce requested for volume-up */
    AdiWrite (PMIC_EIC_BASE + PMIC_EIC_CTRL4, 2U);
  }
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

  SprdGpioInit ();

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gSprdGpioProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSprdGpio
                  );
  DEBUG ((DEBUG_ERROR, "[SprdGpioDxe] Loaded (UMS512)\n"));
  return Status;
}
