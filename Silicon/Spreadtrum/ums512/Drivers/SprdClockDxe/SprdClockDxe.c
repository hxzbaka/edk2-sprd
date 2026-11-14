/** @file
  SprdClockDxe.c

  UMS512 gate-clock implementation of EFI_CLOCK_PROTOCOL.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Protocol/EFIClock.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include "SprdClock.h"

typedef struct {
  CONST CHAR8 *Name;
  UINTN Id;
  UINTN Register;             /* status/read register */
  UINT32 Mask;
} CLOCK_ENTRY;

#define GATE(Name, Id, Base, Offset, Bit) { Name, Id, Base + Offset, BIT##Bit }

STATIC CONST CLOCK_ENTRY mClockTable[] = {
// AON_APB
  GATE ("efuse",    CLK_AON_EFUSE,    AON_APB_BASE, AON_APB_EB1, 0),
  GATE ("gpio",     CLK_AON_GPIO,     AON_APB_BASE, AON_APB_EB1, 1),
  GATE ("kpd",      CLK_AON_KPD,      AON_APB_BASE, AON_APB_EB1, 3),
  GATE ("aon_syst", CLK_AON_AON_SYST, AON_APB_BASE, AON_APB_EB1, 4),
  GATE ("ap_syst",  CLK_AON_AP_SYST,  AON_APB_BASE, AON_APB_EB1, 5),
  GATE ("aon_tmr",  CLK_AON_AON_TMR,  AON_APB_BASE, AON_APB_EB1, 6),
  GATE ("splk",     CLK_AON_SPLK,     AON_APB_BASE, AON_APB_EB1, 10),
  GATE ("pin",      CLK_AON_PIN,      AON_APB_BASE, AON_APB_EB1, 11),
  GATE ("adi",      CLK_AON_ADI,      AON_APB_BASE, AON_APB_EB2, 9),
  GATE ("eic",      CLK_AON_EIC,      AON_APB_BASE, AON_APB_EB2, 10),
  GATE ("i2c",      CLK_AON_APB_I2C,  AON_APB_BASE, AON_APB_EB2, 7),
// AP_AHB
  GATE ("dsi",      CLK_AP_AHB_DSI,   AP_AHB_BASE,  AP_AHB_EB, 0),
  GATE ("dispc",    CLK_AP_AHB_DISPC, AP_AHB_BASE,  AP_AHB_EB, 1),
  GATE ("vsp",      CLK_AP_AHB_VSP,   AP_AHB_BASE,  AP_AHB_EB, 2),
  GATE ("dma-pub",  CLK_AP_AHB_DMA_PUB, AP_AHB_BASE, AP_AHB_EB, 4),
  GATE ("dma",      CLK_AP_AHB_DMA,   AP_AHB_BASE,  AP_AHB_EB, 5),
// AP_AHB
  GATE ("sim0",     CLK_AP_APB_SIM0,  AP_APB_BASE,  AP_APB_EB, 0),
  GATE ("iis0",     CLK_AP_APB_IIS0,  AP_APB_BASE,  AP_APB_EB, 1),
  GATE ("apb_reg",  CLK_AP_APB_APBREG, AP_APB_BASE, AP_APB_EB, 4),
  GATE ("spi0",     CLK_AP_APB_SPI0,  AP_APB_BASE,  AP_APB_EB, 5),
  GATE ("spi1",     CLK_AP_APB_SPI1,  AP_APB_BASE,  AP_APB_EB, 6),
  GATE ("spi2",     CLK_AP_APB_SPI2,  AP_APB_BASE,  AP_APB_EB, 7),
  GATE ("spi3",     CLK_AP_APB_SPI3,  AP_APB_BASE,  AP_APB_EB, 8),
  GATE ("i2c0",     CLK_AP_APB_I2C0,  AP_APB_BASE,  AP_APB_EB, 9),
  GATE ("i2c1",     CLK_AP_APB_I2C1,  AP_APB_BASE,  AP_APB_EB, 10),
  GATE ("i2c2",     CLK_AP_APB_I2C2,  AP_APB_BASE,  AP_APB_EB, 11),
  GATE ("i2c3",     CLK_AP_APB_I2C3,  AP_APB_BASE,  AP_APB_EB, 12),
  GATE ("i2c4",     CLK_AP_APB_I2C4,  AP_APB_BASE,  AP_APB_EB, 13),
  GATE ("uart0",    CLK_AP_APB_UART0, AP_APB_BASE,  AP_APB_EB, 14),
  GATE ("uart1",    CLK_AP_APB_UART1, AP_APB_BASE,  AP_APB_EB, 15),
  GATE ("uart2",    CLK_AP_APB_UART2, AP_APB_BASE,  AP_APB_EB, 16),
  GATE ("sdio0",    CLK_AP_APB_SDIO0, AP_APB_BASE,  AP_APB_EB, 22),
  GATE ("sdio1",    CLK_AP_APB_SDIO1, AP_APB_BASE,  AP_APB_EB, 23),
  GATE ("sdio2",    CLK_AP_APB_SDIO2, AP_APB_BASE,  AP_APB_EB, 24),
  GATE ("emmc",     CLK_AP_APB_EMMC,  AP_APB_BASE,  AP_APB_EB, 25),
  GATE ("sdio0-32k", CLK_AP_APB_SDIO0_32K, AP_APB_BASE, AP_APB_EB, 26),
  GATE ("sdio1-32k", CLK_AP_APB_SDIO1_32K, AP_APB_BASE, AP_APB_EB, 27),
  GATE ("sdio2-32k", CLK_AP_APB_SDIO2_32K, AP_APB_BASE, AP_APB_EB, 28),
  GATE ("emmc-32k",  CLK_AP_APB_EMMC_32K, AP_APB_BASE, AP_APB_EB, 29),
  GATE ("emmc-2x",   CLK_AON_EMMC_2X, AON_APB_BASE, 0x013c, 8),
  GATE ("emmc-1x",   CLK_AON_EMMC_1X, AON_APB_BASE, 0x013c, 9),
  GATE ("otg-utmi",  CLK_AON_OTG_UTMI, AON_APB_BASE, AON_APB_EB1, 8),
  GATE ("otg-phy",   CLK_AON_OTG_PHY, AON_APB_BASE, AON_APB_EB1, 9),
  GATE ("otg-ref",   CLK_AON_OTG_REF, AON_APB_BASE, 0x0138, 12),
};

#define SPRD_SC_GATE_SET_OFFSET    0x1000U
#define SPRD_SC_GATE_CLEAR_OFFSET  0x2000U

STATIC CONST CLOCK_ENTRY *FindClockById (IN UINTN Id) {
  UINTN Index;

  /* IDs are part of the protocol ABI; do not make them depend on table order. */
  for (Index = 0; Index < ARRAY_SIZE (mClockTable); Index++) {
    if (mClockTable[Index].Id == Id) {
      return &mClockTable[Index];
    }
  }
  return NULL;
}
STATIC CONST CLOCK_ENTRY *FindClockByName (IN CONST CHAR8 *Name) {
  UINTN Index;
  if (Name == NULL) return NULL;
  for (Index = 0; Index < ARRAY_SIZE (mClockTable); Index++)
    if (AsciiStrCmp (Name, mClockTable[Index].Name) == 0) return &mClockTable[Index];
  return NULL;
}

STATIC EFI_STATUS EFIAPI
SprdClockGetId (IN EFI_CLOCK_PROTOCOL *This, IN CONST CHAR8 *Name, OUT UINTN *Id) {
  CONST CLOCK_ENTRY *Clock;
  if (Id == NULL || (Clock = FindClockByName (Name)) == NULL) return EFI_NOT_FOUND;
  *Id = Clock->Id;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI
SprdClockEnable (IN EFI_CLOCK_PROTOCOL *This, IN UINTN Id) {
  CONST CLOCK_ENTRY *Clock = FindClockById (Id);
  if (Clock == NULL) return EFI_NOT_FOUND;
  /* SharkL5 gates use write-only set/clear aliases. */
  MmioWrite32 (Clock->Register + SPRD_SC_GATE_SET_OFFSET, Clock->Mask);
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI
SprdClockDisable (IN EFI_CLOCK_PROTOCOL *This, IN UINTN Id) {
  CONST CLOCK_ENTRY *Clock = FindClockById (Id);
  if (Clock == NULL) return EFI_NOT_FOUND;
  MmioWrite32 (Clock->Register + SPRD_SC_GATE_CLEAR_OFFSET, Clock->Mask);
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI
SprdClockIsEnabled (IN EFI_CLOCK_PROTOCOL *This, IN UINTN Id, OUT BOOLEAN *Enabled) {
  CONST CLOCK_ENTRY *Clock = FindClockById (Id);
  if (Clock == NULL || Enabled == NULL) return EFI_INVALID_PARAMETER;
  *Enabled = (MmioRead32 (Clock->Register) & Clock->Mask) != 0;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI Unsupported (VOID) { return EFI_UNSUPPORTED; }

STATIC EFI_CLOCK_PROTOCOL mClockProtocol = {
  EFI_CLOCK_PROTOCOL_VERSION,
  SprdClockGetId, SprdClockEnable, SprdClockDisable, SprdClockIsEnabled,
  (EFI_CLOCK_IS_ON)Unsupported, (EFI_CLOCK_SET_FREQ_HZ)Unsupported,
  (EFI_CLOCK_GET_FREQ_HZ)Unsupported, (EFI_CLOCK_CALC_FREQ_HZ)Unsupported,
  (EFI_CLOCK_SELECT_EXTERNAL_SOURCE)Unsupported, (EFI_POWER_DOMAIN_GET_ID)Unsupported,
  (EFI_POWER_DOMAIN_ENABLE)Unsupported, (EFI_POWER_DOMAIN_DISABLE)Unsupported,
  (EFI_CLOCK_ENTER_LOW_POWER_MODE)Unsupported, (EFI_CLOCK_EXIT_LOW_POWER_MODE)Unsupported,
  (EFI_CLOCK_SET_CLOCK_DIVIDER)Unsupported, (EFI_CLOCK_RESET)Unsupported,
  (EFI_GET_MAX_PERF_LEVEL)Unsupported, (EFI_GET_MIN_PERF_LEVEL)Unsupported,
  (EFI_GET_CPU_PERF_LEVEL_FREQUENCY)Unsupported, (EFI_SET_CPU_PERF_LEVEL)Unsupported,
  (EFI_GET_CPU_PERF_LEVEL)Unsupported, (EFI_GET_CPU_FREQUENCY_LEVELS)Unsupported,
  (EFI_GET_CPU_FREQUECNY_PLAN)Unsupported, (EFI_SET_CPU_FREQUENCY)Unsupported,
  (EFI_GET_CPU_FREQUENCY)Unsupported, (EFI_CLOCK_POST_INIT)Unsupported,
  (EFI_CLOCK_DIVIDER_GET_ID)Unsupported, (EFI_CLOCK_RESET_BRANCH)Unsupported,
  (EFI_CLOCK_INVERT)Unsupported, (EFI_CLOCK_MEASURE_FREQ_HZ)Unsupported,
  (EFI_GET_ID)Unsupported
};

EFI_STATUS EFIAPI
SprdClockDxeInitialize (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;
  Status = gBS->InstallProtocolInterface (&Handle, &gEfiClockProtocolGuid,
                  EFI_NATIVE_INTERFACE, &mClockProtocol);
  if (EFI_ERROR (Status)) DEBUG ((DEBUG_ERROR, "SPRD-Clock: install failed: %r\n", Status));
  return Status;
}