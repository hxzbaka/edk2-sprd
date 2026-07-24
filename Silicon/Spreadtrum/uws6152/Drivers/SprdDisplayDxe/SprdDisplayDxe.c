/******************************************
 * @file
 * Initializes SharkL5 DPU lite_r2p0, SPRD DSI_CTRL and Megacores DPHY
 *
 * Reference    : drivers/gpu/drm/sprd/dpu/dpu_lite_r2p0.c
 *              : drivers/gpu/drm/sprd/dsi/core/dsi_ctrl_r1p0.c
 *              : drivers/gpu/drm/sprd/dphy/pll/megacores_sharkl5.c
 *
 * Copyright (c) 2026 hxzbaka
 ******************************************/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/SprdDisplayPanelLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Uefi.h>

#include "SprdDisplayDxe.h"

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

  Status = SprdDpuGlobalInitialize(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  SprdDpuStop(Config);
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

  SprdDpuConfigureLayer(Config);
  SprdDpuRun(Config);
  return EFI_SUCCESS;
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
  Config.Lanes        = FixedPcdGet8(PcdDsiLaneCount);
  Config.CommandMode  = FixedPcdGetBool(PcdDisplayCmdMode);

  if ((Config.DpuBase == 0) || (Config.DsiBase == 0) ||
      (Config.ApAhbBase == 0) || (Config.ApClkBase == 0) ||
      (Config.PmuApbBase == 0) || (Config.FrameBuffer == 0) ||
      (Config.Width == 0) || (Config.Height == 0) ||
      (Config.PixelClock == 0) || (Config.PhyBitClock == 0) ||
      (Config.EscapeClock == 0) || (Config.Lanes == 0) ||
      !Config.CommandMode) {
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
