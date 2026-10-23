/** @file
  Display clock and reset initialization

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdDisplayDxe.h"

#define AP_AHB_AHB_RST        0x0004
#define AP_AHB_MISC_CKG_EN    0x0040
#define AP_AHB_GATE_SET       0x1000
#define AP_AHB_DSI_EB         BIT0
#define AP_AHB_DISPC_EB       BIT1
#define AP_AHB_DSI_RESET      BIT0
#define AP_AHB_DISPC_RESET    BIT1
#define AP_AHB_DPHY_ENABLE    (BIT0 | BIT1)
#define AP_CLK_DISPC0_CFG      0x00A0
#define AP_CLK_DISPC0_DPI_CFG  0x00A4
#define AP_CLK_DISPC0_SEL_MASK 0x3U
#define AP_CLK_DISPC0_SEL_384M 0x3U
#define AP_CLK_DPI_SEL_MASK    0x3U
#define AP_CLK_DPI_DIV_MASK    (0xFU << 8)

#define PMU_APB_DPHY_POWER_CFG 0x035C
#define PMU_APB_DPHY_POWER_OFF BIT3
#define AON_APB_PWR_CTRL       0x0024
#define AON_APB_DSI_PS_PD_S    BIT15
#define AON_APB_DSI_PS_PD_L    BIT14

STATIC
EFI_STATUS
ConfigureDpiClock(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  STATIC CONST UINT32 Sources[] = { 96000000U, 128000000U, 153600000U };
  UINT32              Divider;
  UINT32              Difference;
  UINT32              Index;
  UINT32              BestDifference;
  UINT32              BestDivider;
  UINT32              BestIndex;
  UINT32              Rate;

  BestDifference = MAX_UINT32;
  BestDivider = 0;
  BestIndex = 0;

  for (Index = 0; Index < ARRAY_SIZE(Sources); Index++) {
    for (Divider = 1; Divider <= 16; Divider++) {
      Rate = Sources[Index] / Divider;
      Difference = (Rate > Config->PixelClock) ?
          Rate - Config->PixelClock : Config->PixelClock - Rate;
      if (Difference < BestDifference) {
        BestDifference = Difference;
        BestDivider = Divider;
        BestIndex = Index;
      }
    }
  }

  if (BestDifference > Config->PixelClock / 100) {
    return EFI_UNSUPPORTED;
  }

  MmioAndThenOr32(
      Config->ApClkBase + AP_CLK_DISPC0_DPI_CFG,
      ~(AP_CLK_DPI_SEL_MASK | AP_CLK_DPI_DIV_MASK),
      BestIndex | ((BestDivider - 1) << 8));
  return EFI_SUCCESS;
}

EFI_STATUS
SprdDpuGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  EFI_STATUS Status;

  MmioWrite32(Config->ApAhbBase + AP_AHB_GATE_SET, AP_AHB_DISPC_EB);
  MmioAndThenOr32(
      Config->ApClkBase + AP_CLK_DISPC0_CFG,
      ~AP_CLK_DISPC0_SEL_MASK,
      AP_CLK_DISPC0_SEL_384M);

  Status = ConfigureDpiClock(Config);
  return Status;
}

VOID
SprdDpuGlobalReset(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioOr32(Config->ApAhbBase + AP_AHB_AHB_RST, AP_AHB_DISPC_RESET);
  gBS->Stall(10);
  MmioAnd32(Config->ApAhbBase + AP_AHB_AHB_RST, ~AP_AHB_DISPC_RESET);
}

VOID
SprdDsiGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioWrite32(Config->ApAhbBase + AP_AHB_GATE_SET, AP_AHB_DSI_EB);

  MmioOr32(Config->ApAhbBase + AP_AHB_AHB_RST, AP_AHB_DSI_RESET);
  gBS->Stall(10);
  MmioAnd32(Config->ApAhbBase + AP_AHB_AHB_RST, ~AP_AHB_DSI_RESET);

}

VOID
SprdDphyGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioAnd32(
      Config->PmuApbBase + PMU_APB_DPHY_POWER_CFG,
      ~PMU_APB_DPHY_POWER_OFF);
  gBS->Stall(100);
  MmioAnd32(FixedPcdGet32(PcdAonApbBaseAddress) + AON_APB_PWR_CTRL, ~AON_APB_DSI_PS_PD_S);
  gBS->Stall(10);
  MmioAnd32(FixedPcdGet32(PcdAonApbBaseAddress) + AON_APB_PWR_CTRL, ~AON_APB_DSI_PS_PD_L);
  MmioOr32(Config->ApAhbBase + AP_AHB_MISC_CKG_EN, AP_AHB_DPHY_ENABLE);
}
