/** @file
  SprdDisplayDxe.h

  Display driver type definitions and function declarations

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef SPRD_DISPLAY_DXE_H_
#define SPRD_DISPLAY_DXE_H_

#include <Uefi.h>

typedef struct {
  UINTN   DpuBase;
  UINTN   DsiBase;
  UINTN   ApAhbBase;
  UINTN   ApClkBase;
  UINTN   PmuApbBase;
  UINT32  FrameBuffer;
  UINT32  Width;
  UINT32  Height;
  UINT32  PixelClock;
  UINT32  PhyBitClock;
  UINT32  EscapeClock;
  UINT32  HSync;
  UINT32  HBackPorch;
  UINT32  HFrontPorch;
  UINT32  VSync;
  UINT32  VBackPorch;
  UINT32  VFrontPorch;
  UINT8   Lanes;
  UINT8   VideoModeType;
  BOOLEAN CommandMode;
  BOOLEAN VideoMode;
} SPRD_DISPLAY_CONFIG;

EFI_STATUS
SprdDpuGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDpuGlobalReset(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDsiGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDphyGlobalInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDpuInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDpuConfigureLayer(IN CONST SPRD_DISPLAY_CONFIG *Config);

EFI_STATUS
SprdDpuRun(IN CONST SPRD_DISPLAY_CONFIG *Config);

EFI_STATUS
SprdDsiInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDsiEnableLpCommands(IN CONST SPRD_DISPLAY_CONFIG *Config);

VOID
SprdDsiPrepareForTransfer(IN CONST SPRD_DISPLAY_CONFIG *Config);

EFI_STATUS
SprdDsiGenericWrite(
    IN CONST SPRD_DISPLAY_CONFIG *Config,
    IN CONST UINT8               *Payload,
    IN UINTN                      PayloadLength);

EFI_STATUS
SprdDphyInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config);

EFI_STATUS
SprdDphyEnableHsClock(IN CONST SPRD_DISPLAY_CONFIG *Config);

#endif