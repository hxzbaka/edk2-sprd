/** @file
  SprdDisplayDpu.c

  SC9832E DPU lite r1p0 initialization

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdDisplayDxe.h"

#define DPU_CTRL          0x0004
#define DPU_SIZE          0x0008
#define DPU_QOS           0x0014
#define DPU_BG_COLOR      0x001C
#define DPU_LAYER_BASE(Index)  (0x0020 + ((Index) * 0x0030))
#define DPU_LAYER_ADDR0(Index) (DPU_LAYER_BASE(Index) + 0x0000)
#define DPU_LAYER_CTRL(Index)  (DPU_LAYER_BASE(Index) + 0x0010)
#define DPU_LAYER_SIZE(Index)  (DPU_LAYER_BASE(Index) + 0x0014)
#define DPU_LAYER_PITCH(Index) (DPU_LAYER_BASE(Index) + 0x0018)
#define DPU_LAYER_POS(Index)   (DPU_LAYER_BASE(Index) + 0x001C)
#define DPU_LAYER_ALPHA(Index) (DPU_LAYER_BASE(Index) + 0x0020)
#define DPU_LAYER_CROP(Index)  (DPU_LAYER_BASE(Index) + 0x002C)
#define DPU_INT_EN        0x0160
#define DPU_INT_CLR       0x0164
#define DPU_INT_RAW       0x016C
#define DPU_DPI_CTRL      0x0170
#define DPU_DPI_H_TIMING  0x0174
#define DPU_DPI_V_TIMING  0x0178
#define DPU_MMU_EN        0x0800

#define DPU_CTRL_RUN          BIT0
#define DPU_CTRL_STOP         BIT1
#define DPU_CTRL_EDPI         BIT2
#define DPU_CTRL_SW_UPDATE    BIT4
#define DPU_CTRL_UPDATE       BIT5
#define DPU_CTRL_DITHER       BIT6
#define DPU_CTRL_POWER        BIT7
#define DPU_INT_DONE          BIT0
#define DPU_DPI_RGB888        (2U << 6)
#define DPU_DPI_TE_ENABLE     BIT8
#define DPU_DPI_TE_EXTERNAL   BIT10
#define DPU_LAYER_ENABLE      BIT0
#define DPU_LAYER_BLOCK_ALPHA BIT2
#define DPU_LAYER_ARGB8888    (2U << 4)
#define DPU_LAYER0_CTRL       \
  (DPU_LAYER_ENABLE | DPU_LAYER_BLOCK_ALPHA | DPU_LAYER_ARGB8888)
#define DPU_DEFAULT_QOS       0x0004E6EBU

STATIC
VOID
DpuCleanLayers(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Index;

  for (Index = 0; Index < 6; Index++) {
    MmioWrite32(Config->DpuBase + DPU_LAYER_CTRL(Index), 0);
  }
}

VOID
SprdDpuInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Size;

  Size = (Config->Height << 16) | Config->Width;
  MmioWrite32(Config->DpuBase + DPU_INT_EN, 0);
  MmioWrite32(Config->DpuBase + DPU_INT_CLR, 0xFFFF);
  MmioWrite32(Config->DpuBase + DPU_MMU_EN, 0);
  MmioWrite32(Config->DpuBase + DPU_BG_COLOR, 0);
  MmioWrite32(Config->DpuBase + DPU_SIZE, Size);
  MmioWrite32(Config->DpuBase + DPU_QOS, DPU_DEFAULT_QOS);
  DpuCleanLayers(Config);

  if (Config->VideoMode) {
    MmioAndThenOr32(
        Config->DpuBase + DPU_CTRL,
        ~(DPU_CTRL_STOP | DPU_CTRL_EDPI),
        DPU_CTRL_DITHER | DPU_CTRL_POWER);
    MmioWrite32(Config->DpuBase + DPU_DPI_CTRL, DPU_DPI_RGB888);
    MmioWrite32(
        Config->DpuBase + DPU_DPI_H_TIMING,
        Config->HSync | (Config->HBackPorch << 8) |
        (Config->HFrontPorch << 20));
    MmioWrite32(
        Config->DpuBase + DPU_DPI_V_TIMING,
        Config->VSync | (Config->VBackPorch << 8) |
        (Config->VFrontPorch << 20));
  } else {
    MmioAndThenOr32(
        Config->DpuBase + DPU_CTRL,
        ~DPU_CTRL_STOP,
        DPU_CTRL_EDPI | DPU_CTRL_DITHER | DPU_CTRL_POWER);
    MmioWrite32(
        Config->DpuBase + DPU_DPI_CTRL,
        DPU_DPI_RGB888 | DPU_DPI_TE_ENABLE | DPU_DPI_TE_EXTERNAL);
  }
}

VOID
SprdDpuConfigureLayer(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Size;

  Size = (Config->Height << 16) | Config->Width;
  DpuCleanLayers(Config);
  MmioWrite32(Config->DpuBase + DPU_BG_COLOR, 0);
  // With the MMU disabled, DPU addresses DDR relative to the CPU RAM base.
  MmioWrite32(
      Config->DpuBase + DPU_LAYER_ADDR0(0),
      (UINT32)(Config->FrameBuffer - FixedPcdGet64(PcdSystemMemoryBase)));
  MmioWrite32(Config->DpuBase + DPU_LAYER_SIZE(0), Size);
  MmioWrite32(Config->DpuBase + DPU_LAYER_PITCH(0), Config->Width);
  MmioWrite32(Config->DpuBase + DPU_LAYER_POS(0), 0);
  MmioWrite32(Config->DpuBase + DPU_LAYER_ALPHA(0), 0xFF);
  MmioWrite32(Config->DpuBase + DPU_LAYER_CROP(0), 0);
  MmioWrite32(Config->DpuBase + DPU_LAYER_CTRL(0), DPU_LAYER0_CTRL);
}

EFI_STATUS
SprdDpuRun(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Retry;

  if (Config->VideoMode) {
    MmioOr32(Config->DpuBase + DPU_CTRL, DPU_CTRL_SW_UPDATE);
    MmioOr32(Config->DpuBase + DPU_CTRL, DPU_CTRL_UPDATE);
    gBS->Stall(10);
    MmioOr32(Config->DpuBase + DPU_CTRL, DPU_CTRL_RUN);
    MmioAnd32(Config->DpuBase + DPU_CTRL, ~DPU_CTRL_SW_UPDATE);
    return EFI_SUCCESS;
  }

  MmioWrite32(Config->DpuBase + DPU_INT_CLR, DPU_INT_DONE);
  MmioOr32(Config->DpuBase + DPU_CTRL, DPU_CTRL_RUN);
  for (Retry = 0; Retry < 500; Retry++) {
    if ((MmioRead32(Config->DpuBase + DPU_INT_RAW) & DPU_INT_DONE) != 0) {
      MmioWrite32(Config->DpuBase + DPU_INT_CLR, DPU_INT_DONE);
      return EFI_SUCCESS;
    }

    gBS->Stall(1000);
  }

  return EFI_TIMEOUT;
}