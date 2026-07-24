#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdDisplayDxe.h"

#define AP_AHB_AHB_RST        0x0004
#define AP_AHB_MISC_CKG_EN    0x0040
#define AP_AHB_AP_QOS2        0x00BC
#define AP_AHB_GATE_SET       0x1000
#define AP_AHB_DSI_EB         BIT0
#define AP_AHB_DISPC_EB       BIT1
#define AP_AHB_DSI_RESET      BIT0
#define AP_AHB_DISPC_RESET    BIT1
#define AP_AHB_DPHY_ENABLE    (BIT0 | BIT1)
#define AP_AHB_DPU_QOS_MASK   0xFFU
#define AP_AHB_DPU_QOS_VALUE  0xCFU

#define AP_CLK_DISPC0_CFG      0x0098
#define AP_CLK_DISPC0_DPI_CFG  0x009C
#define AP_CLK_DISPC0_SEL_MASK 0x7U
#define AP_CLK_DISPC0_SEL_384M 0x4U
#define AP_CLK_DPI_SEL_MASK    0x3U
#define AP_CLK_DPI_DIV_MASK    (0xFU << 8)

#define PMU_APB_DPHY_POWER_CFG 0x035C
#define PMU_APB_DPHY_POWER_OFF BIT3

STATIC
EFI_STATUS
ConfigureDpiClock(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  STATIC CONST UINT32 Sources[] = { 96000000U, 128000000U, 153600000U };
  UINT32              Divider;
  UINT32              Index;

  for (Index = 0; Index < ARRAY_SIZE(Sources); Index++) {
    if ((Sources[Index] % Config->PixelClock) != 0) {
      continue;
    }

    Divider = Sources[Index] / Config->PixelClock;
    if ((Divider == 0) || (Divider > 16)) {
      continue;
    }

    MmioAndThenOr32(
        Config->ApClkBase + AP_CLK_DISPC0_DPI_CFG,
        ~(AP_CLK_DPI_SEL_MASK | AP_CLK_DPI_DIV_MASK),
        Index | ((Divider - 1) << 8));
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
SprdDpuGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  EFI_STATUS Status;

  MmioWrite32(Config->ApAhbBase + AP_AHB_GATE_SET, AP_AHB_DISPC_EB);
  MmioAndThenOr32(
      Config->ApAhbBase + AP_AHB_AP_QOS2,
      ~AP_AHB_DPU_QOS_MASK,
      AP_AHB_DPU_QOS_VALUE);

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
  MmioOr32(Config->ApAhbBase + AP_AHB_MISC_CKG_EN, AP_AHB_DPHY_ENABLE);
}
