/** @file
  SprdDisplayDpu.c

  DPU lite_r2p0 initialization and layer configuration

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdDisplayDxe.h"

#define DPU_CTRL        0x0004
#define DPU_CFG0        0x0008
#define DPU_CFG1        0x000C
#define DPU_CFG2        0x0010
#define DPU_PANEL_SIZE  0x0020
#define DPU_BLEND_SIZE  0x0024
#define DPU_BG_COLOR    0x002C

#define DPU_LAYER_BASE(Index)   (0x0030 + ((Index) * 0x0030))
#define DPU_LAYER_ADDR0(Index)  (DPU_LAYER_BASE(Index) + 0x0000)
#define DPU_LAYER_CTRL(Index)   (DPU_LAYER_BASE(Index) + 0x0010)
#define DPU_LAYER_SIZE(Index)   (DPU_LAYER_BASE(Index) + 0x0014)
#define DPU_LAYER_PITCH(Index)  (DPU_LAYER_BASE(Index) + 0x0018)
#define DPU_LAYER_POS(Index)    (DPU_LAYER_BASE(Index) + 0x001C)
#define DPU_LAYER_ALPHA(Index)  (DPU_LAYER_BASE(Index) + 0x0020)
#define DPU_LAYER_CROP(Index)   (DPU_LAYER_BASE(Index) + 0x002C)

#define DPU_INT_EN      0x01E0
#define DPU_INT_CLR     0x01E4
#define DPU_INT_RAW     0x01EC
#define DPU_DPI_CTRL    0x01F0
#define DPU_DPI_H_TIMING 0x01F4
#define DPU_DPI_V_TIMING 0x01F8
#define DPU_ENHANCE_CFG 0x0200
#define DPU_SLP_LUT_ADDR  0x02D0
#define DPU_SLP_LUT_WDATA 0x02D4

#define DPU_MMU_EN         0x0800
#define DPU_MMU_VPN_RANGE  0x080C
#define DPU_MMU_MIN_PPN1   0x083C
#define DPU_MMU_PPN_RANGE1 0x0840
#define DPU_MMU_MIN_PPN2   0x0844
#define DPU_MMU_PPN_RANGE2 0x0848

#define DPU_CTRL_RUN          BIT0
#define DPU_CTRL_STOP         BIT1
#define DPU_INT_DONE           BIT0
#define DPU_CFG0_EDPI          BIT0
#define DPU_DPI_TE_ENABLE      BIT8
#define DPU_DPI_TE_EXTERNAL    BIT10
#define DPU_DPI_HALT_ENABLE    BIT16
#define DPU_LAYER_ENABLE       BIT0
#define DPU_LAYER_ALPHA_LAYER  BIT2
#define DPU_LAYER_ARGB8888     (3U << 4)
#define DPU_LAYER0_CTRL        \
  (DPU_LAYER_ENABLE | DPU_LAYER_ALPHA_LAYER | DPU_LAYER_ARGB8888)

#define DPU_CFG1_QOS 0x0044AADCU
#define DPU_CFG2_VALUE 0x00014002U

STATIC CONST UINT16 SlpLut[256] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1,
  0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
  0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
  0x3, 0x3, 0x3, 0x3, 0x4, 0x4, 0x4, 0x4,
  0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x7, 0x7,
  0x8, 0x8, 0x9, 0x9, 0xA, 0xA, 0xB, 0xC,
  0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13,
  0x14, 0x15, 0x16, 0x18, 0x19, 0x1B, 0x1C, 0x1E,
  0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2D, 0x2F,
  0x32, 0x35, 0x38, 0x3B, 0x3F, 0x42, 0x46, 0x4A,
  0x4E, 0x52, 0x57, 0x5C, 0x61, 0x66, 0x6B, 0x71,
  0x77, 0x7E, 0x84, 0x8B, 0x92, 0x9A, 0xA2, 0xAA,
  0xB2, 0xBB, 0xC4, 0xCE, 0xD8, 0xE2, 0xEC, 0xF7,
  0x102, 0x10E, 0x11A, 0x126, 0x132, 0x13F, 0x14C, 0x159,
  0x167, 0x175, 0x183, 0x191, 0x19F, 0x1AE, 0x1BC, 0x1CB,
  0x1DA, 0x1E9, 0x1F8, 0x207, 0x216, 0x225, 0x234, 0x243,
  0x251, 0x260, 0x26E, 0x27C, 0x28A, 0x298, 0x2A6, 0x2B3,
  0x2C0, 0x2CD, 0x2D9, 0x2E5, 0x2F1, 0x2FD, 0x308, 0x313,
  0x31D, 0x327, 0x331, 0x33B, 0x344, 0x34D, 0x355, 0x35D,
  0x365, 0x36D, 0x374, 0x37B, 0x381, 0x388, 0x38E, 0x394,
  0x399, 0x39E, 0x3A3, 0x3A8, 0x3AD, 0x3B1, 0x3B5, 0x3B9,
  0x3BD, 0x3C0, 0x3C4, 0x3C7, 0x3CA, 0x3CD, 0x3D0, 0x3D2,
  0x3D5, 0x3D7, 0x3D9, 0x3DB, 0x3DD, 0x3DF, 0x3E1, 0x3E3,
  0x3E4, 0x3E6, 0x3E7, 0x3E9, 0x3EA, 0x3EB, 0x3EC, 0x3ED,
  0x3EE, 0x3EF, 0x3F0, 0x3F1, 0x3F2, 0x3F3, 0x3F3, 0x3F4
};

STATIC
VOID
DpuPollRaw(IN CONST SPRD_DISPLAY_CONFIG *Config, IN UINT32 Mask, IN UINT32 TimeoutMs)
{
  while (TimeoutMs-- != 0) {
    if ((MmioRead32(Config->DpuBase + DPU_INT_RAW) & Mask) != 0) {
      return;
    }

    gBS->Stall(1000);
  }

}

STATIC
VOID
DpuCleanLayers(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Index;

  for (Index = 0; Index < 8; Index++) {
    MmioWrite32(Config->DpuBase + DPU_LAYER_CTRL(Index), 0);
  }
}

STATIC
VOID
DpuReloadEnhance(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Index;

  for (Index = 0; Index < ARRAY_SIZE(SlpLut); Index++) {
    MmioWrite32(Config->DpuBase + DPU_SLP_LUT_ADDR, Index);
    gBS->Stall(1);
    MmioWrite32(Config->DpuBase + DPU_SLP_LUT_WDATA, SlpLut[Index]);
  }

  MmioWrite32(Config->DpuBase + DPU_ENHANCE_CFG, 0);
}

VOID
SprdDpuStop(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioWrite32(Config->DpuBase + DPU_INT_EN, 0);

  if ((MmioRead32(Config->DpuBase + DPU_CTRL) & DPU_CTRL_RUN) == 0) {
    return;
  }

  if (!Config->CommandMode) {
    MmioWrite32(Config->DpuBase + DPU_INT_CLR, DPU_INT_DONE);
    MmioOr32(Config->DpuBase + DPU_CTRL, DPU_CTRL_STOP);
  }

  DpuPollRaw(Config, DPU_INT_DONE, 500);
  MmioWrite32(Config->DpuBase + DPU_INT_CLR, DPU_INT_DONE);
}

VOID
SprdDpuInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Size;

  Size = (Config->Height << 16) | Config->Width;
  MmioWrite32(Config->DpuBase + DPU_BG_COLOR, 0);
  MmioWrite32(Config->DpuBase + DPU_PANEL_SIZE, Size);
  MmioWrite32(Config->DpuBase + DPU_BLEND_SIZE, Size);
  MmioWrite32(Config->DpuBase + DPU_CFG0, 0);
  MmioWrite32(Config->DpuBase + DPU_CFG1, DPU_CFG1_QOS);
  MmioWrite32(Config->DpuBase + DPU_CFG2, DPU_CFG2_VALUE);

  DpuCleanLayers(Config);

  MmioWrite32(Config->DpuBase + DPU_MMU_EN, 0);
  MmioWrite32(Config->DpuBase + DPU_MMU_MIN_PPN1, 0);
  MmioWrite32(Config->DpuBase + DPU_MMU_PPN_RANGE1, 0xFFFF);
  MmioWrite32(Config->DpuBase + DPU_MMU_MIN_PPN2, 0);
  MmioWrite32(Config->DpuBase + DPU_MMU_PPN_RANGE2, 0xFFFF);
  MmioWrite32(Config->DpuBase + DPU_MMU_VPN_RANGE, 0x1FFFF);
  MmioWrite32(Config->DpuBase + DPU_INT_EN, 0);
  MmioWrite32(Config->DpuBase + DPU_INT_CLR, 0xFFFF);

  DpuReloadEnhance(Config);

  if (!Config->VideoMode) {
    MmioOr32(Config->DpuBase + DPU_CFG0, DPU_CFG0_EDPI);
    MmioOr32(
        Config->DpuBase + DPU_DPI_CTRL,
        DPU_DPI_TE_ENABLE | DPU_DPI_TE_EXTERNAL);
  } else {
    MmioAnd32(Config->DpuBase + DPU_CFG0, ~DPU_CFG0_EDPI);
    MmioAnd32(Config->DpuBase + DPU_DPI_CTRL, ~DPU_DPI_HALT_ENABLE);
    MmioWrite32(
        Config->DpuBase + DPU_DPI_H_TIMING,
        Config->HSync | (Config->HBackPorch << 8) |
        (Config->HFrontPorch << 20));
    MmioWrite32(
        Config->DpuBase + DPU_DPI_V_TIMING,
        Config->VSync | (Config->VBackPorch << 8) |
        (Config->VFrontPorch << 20));
  }
}

VOID
SprdDpuConfigureLayer(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Size;

  Size = (Config->Height << 16) | Config->Width;
  DpuCleanLayers(Config);
  MmioWrite32(Config->DpuBase + DPU_BG_COLOR, 0);
  MmioWrite32(Config->DpuBase + DPU_LAYER_ADDR0(0), Config->FrameBuffer);
  MmioWrite32(Config->DpuBase + DPU_LAYER_SIZE(0), Size);
  MmioWrite32(Config->DpuBase + DPU_LAYER_PITCH(0), Config->Width);
  MmioWrite32(Config->DpuBase + DPU_LAYER_POS(0), 0);
  MmioWrite32(Config->DpuBase + DPU_LAYER_ALPHA(0), 0xFF);
  MmioWrite32(Config->DpuBase + DPU_LAYER_CROP(0), 0);
  MmioWrite32(Config->DpuBase + DPU_LAYER_CTRL(0), DPU_LAYER0_CTRL);
}

VOID
SprdDpuRun(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  if (!Config->VideoMode) {
    MmioWrite32(Config->DpuBase + DPU_INT_CLR, DPU_INT_DONE);
  }

  MmioOr32(Config->DpuBase + DPU_CTRL, DPU_CTRL_RUN);

  if (!Config->VideoMode) {
    DpuPollRaw(Config, DPU_INT_DONE, 500);
    MmioWrite32(Config->DpuBase + DPU_INT_CLR, DPU_INT_DONE);
    gBS->Stall(20000);
  }
}