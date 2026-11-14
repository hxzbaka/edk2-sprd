/** @file
  SprdDisplayDxe.c

  Initializes SC9832E DPU lite r1p0, DSI and Megacores DPHY

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Library/BaseLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SprdDisplayPanelLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Uefi.h>

#include "SprdDisplayDxe.h"

#define DPU_LAYER0_CTRL        0x0030
#define DPU_LAYER_FORMAT_MASK  (0xFU << 4)
#define DPU_LAYER_RGB565       (3U << 4)

STATIC
EFI_STATUS
InstallDisplayReadyProtocol(VOID)
{
  EFI_HANDLE Handle;

  Handle = NULL;
  return gBS->InstallProtocolInterface(
      &Handle,
      &gSprdDisplayReadyProtocolGuid,
      EFI_NATIVE_INTERFACE,
      NULL);
}

STATIC
EFI_STATUS
EFIAPI
DisplayDsiWrite(
    IN CONST VOID  *Context,
    IN CONST UINT8 *Payload,
    IN UINTN        PayloadLength)
{
  return SprdDsiGenericWrite(Context, Payload, PayloadLength);
}

STATIC
EFI_STATUS
InitializeDisplayHardware(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  SPRD_DISPLAY_PANEL_IO PanelIo;
  EFI_STATUS Status;
  UINT32 LayerControl;
  BOOLEAN ConvertRgb565;

  LayerControl = MmioRead32(Config->DpuBase + DPU_LAYER0_CTRL);
  ConvertRgb565 = (LayerControl & (BIT0 | DPU_LAYER_FORMAT_MASK)) ==
      (BIT0 | DPU_LAYER_RGB565);

  Status = SprdDpuGlobalInitialize(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  SprdDpuGlobalReset(Config);
  SprdDpuInitialize(Config);

  SprdDsiGlobalInitialize(Config);
  Status = SprdDsiInitialize(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  SprdDphyGlobalInitialize(Config);
  Status = SprdDphyInitialize(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  SprdDsiEnableLpCommands(Config);
  PanelIo.Context = Config;
  PanelIo.Write   = DisplayDsiWrite;
  Status = SprdDisplayPanelPrepare(&PanelIo);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = SprdDisplayPanelEnable(&PanelIo);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  SprdDsiPrepareForTransfer(Config);
  Status = SprdDphyEnableHsClock(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ConvertRgb565) {
    UINT16 *Source = (UINT16 *)(UINTN)Config->FrameBuffer;
    UINT32 *Destination = (UINT32 *)(UINTN)Config->FrameBuffer;
    UINTN PixelIndex = (UINTN)Config->Width * Config->Height;
    UINT16 Pixel;
    UINT32 Red;
    UINT32 Green;
    UINT32 Blue;

    // Expand backwards so BGRA writes cannot overwrite unread RGB565 pixels.
    while (PixelIndex > 0) {
      PixelIndex--;
      Pixel = Source[PixelIndex];
      Red = (Pixel >> 11) & 0x1FU;
      Green = (Pixel >> 5) & 0x3FU;
      Blue = Pixel & 0x1FU;
      Red = (Red << 3) | (Red >> 2);
      Green = (Green << 2) | (Green >> 4);
      Blue = (Blue << 3) | (Blue >> 2);
      Destination[PixelIndex] = (Red << 16) | (Green << 8) | Blue;
    }
  }

  WriteBackDataCacheRange(
      (VOID *)(UINTN)Config->FrameBuffer,
      (UINTN)Config->Width * Config->Height * sizeof(UINT32));
  SprdDpuConfigureLayer(Config);
  return SprdDpuRun(Config);
}

EFI_STATUS
EFIAPI
SprdDisplayDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  SPRD_DISPLAY_CONFIG Config;
  EFI_STATUS          Status;

  if (!FixedPcdGetBool(PcdReconfigureDisplay)) {
    Status = InstallDisplayReadyProtocol();
    ASSERT_EFI_ERROR(Status);
    return Status;
  }

  Config.DpuBase      = FixedPcdGet32(PcdDpuBaseAddress);
  Config.DsiBase      = FixedPcdGet32(PcdDsiBaseAddress);
  Config.ApAhbBase    = FixedPcdGet32(PcdApAhbBaseAddress);
  Config.ApClkBase    = FixedPcdGet32(PcdApClkBaseAddress);
  Config.PmuApbBase   = FixedPcdGet32(PcdPmuApbBaseAddress);
  Config.FrameBuffer  = FixedPcdGet32(PcdMipiFrameBufferAddress);
  Config.Width        = FixedPcdGet32(PcdMipiFrameBufferWidth);
  Config.Height       = FixedPcdGet32(PcdMipiFrameBufferHeight);
  Config.PixelClock   = FixedPcdGet32(PcdMipiPixelClock);
  Config.PhyBitClock  = FixedPcdGet32(PcdDsiPhyBitClock);
  Config.EscapeClock  = FixedPcdGet32(PcdDsiEscapeClock);
  Config.HSync        = FixedPcdGet32(PcdDisplayHSync);
  Config.HBackPorch   = FixedPcdGet32(PcdDisplayHBackPorch);
  Config.HFrontPorch  = FixedPcdGet32(PcdDisplayHFrontPorch);
  Config.VSync        = FixedPcdGet32(PcdDisplayVSync);
  Config.VBackPorch   = FixedPcdGet32(PcdDisplayVBackPorch);
  Config.VFrontPorch  = FixedPcdGet32(PcdDisplayVFrontPorch);
  Config.Lanes        = FixedPcdGet8(PcdDsiLaneCount);
  Config.VideoModeType = FixedPcdGet8(PcdDisplayVideoModeType);
  Config.CommandMode  = FixedPcdGetBool(PcdDisplayCmdMode);
  Config.VideoMode    = FixedPcdGetBool(PcdDisplaySwitchToVideoMode) ||
      !Config.CommandMode;

  if ((Config.DpuBase == 0) || (Config.DsiBase == 0) ||
      (Config.ApAhbBase == 0) || (Config.ApClkBase == 0) ||
      (Config.PmuApbBase == 0) || (Config.FrameBuffer == 0) ||
      (Config.Width == 0) || (Config.Height == 0) ||
      (Config.PixelClock == 0) || (Config.PhyBitClock == 0) ||
      (Config.EscapeClock == 0) || (Config.Lanes == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Config.VideoMode &&
      ((Config.HSync == 0) || (Config.VSync == 0) ||
       (Config.VideoModeType > 3))) {
    return EFI_INVALID_PARAMETER;
  }

  Status = InitializeDisplayHardware(&Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = InstallDisplayReadyProtocol();
  ASSERT_EFI_ERROR(Status);
  return Status;
}