#include <Guid/EventGroup.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/FrameBufferBltLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <PiDxe.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HardwareInterrupt.h>
#include <Uefi.h>

typedef struct {
  VENDOR_DEVICE_PATH DisplayDevicePath;
  EFI_DEVICE_PATH    EndDevicePath;
} DISPLAY_DEVICE_PATH;

DISPLAY_DEVICE_PATH mDisplayDevicePath = {
    {{HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
          (UINT8)(sizeof(VENDOR_DEVICE_PATH)),
          (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8),
      }},
     EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID},
    {END_DEVICE_PATH_TYPE,
     END_ENTIRE_DEVICE_PATH_SUBTYPE,
     {sizeof(EFI_DEVICE_PATH_PROTOCOL), 0}}};

STATIC FRAME_BUFFER_CONFIGURE *mFrameBufferBltLibConfigure;
STATIC UINTN                   mFrameBufferBltLibConfigureSize;
STATIC EFI_EVENT                       mExitBootServicesEvent;
STATIC EFI_HARDWARE_INTERRUPT_PROTOCOL *mHardwareInterrupt;
STATIC BOOLEAN                         mDpuInterruptRegistered;
STATIC BOOLEAN                         mDpuFrameInFlight;

#define DPU_INT_ERROR_MASK BIT2
#define DPU_INT_RAW        0x016C

STATIC
EFI_STATUS
EFIAPI
DisplayQueryMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);

STATIC
EFI_STATUS
EFIAPI
DisplaySetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber);

STATIC
EFI_STATUS
EFIAPI
DisplayBlt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *            BltBuffer,
    OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX,
    IN UINTN DestinationY, IN UINTN Width, IN UINTN Height,
    IN UINTN Delta OPTIONAL);

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL mDisplay = {
    DisplayQueryMode, DisplaySetMode, DisplayBlt, NULL};

STATIC VOID EdpiTriggerFrame(VOID)
{
  UINTN DpuBase = FixedPcdGet32(PcdDpuBaseAddress);

  if (mDpuFrameInFlight) {
    return;
  }

  MmioWrite32(
      DpuBase + FixedPcdGet32(PcdDpuIntClrReg),
      FixedPcdGet32(PcdDpuIntDoneBit));
  mDpuFrameInFlight = TRUE;
  MmioOr32(DpuBase + FixedPcdGet32(PcdDpuCtrlReg), BIT0);
}

STATIC EFI_STATUS EdpiWaitFrameDone(VOID)
{
  UINTN DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 Retry;

  if (!mDpuFrameInFlight) {
    return EFI_SUCCESS;
  }

  for (Retry = 0; Retry < 500; Retry++) {
    if ((MmioRead32(DpuBase + DPU_INT_RAW) &
         FixedPcdGet32(PcdDpuIntDoneBit)) != 0) {
      mDpuFrameInFlight = FALSE;
      return EFI_SUCCESS;
    }

    gBS->Stall(1000);
  }

  return EFI_TIMEOUT;
}

STATIC VOID EdpiDisableInterrupts(VOID)
{
  UINTN DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 IntMask = FixedPcdGet32(PcdDpuIntDoneBit) |
      FixedPcdGet32(PcdDpuIntTeBit) | DPU_INT_ERROR_MASK;

  MmioAnd32(DpuBase + FixedPcdGet32(PcdDpuIntEnReg), ~IntMask);
  MmioWrite32(DpuBase + FixedPcdGet32(PcdDpuIntClrReg), IntMask);
}

STATIC VOID EdpiConfigureTe(VOID)
{
  UINTN DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 TeBits = FixedPcdGet32(PcdDpuDpiCtrlTeEnableBit) |
      FixedPcdGet32(PcdDpuDpiCtrlTeExternalBit);

  MmioOr32(DpuBase + FixedPcdGet32(PcdDpuDpiCtrlReg), TeBits);
  MmioOr32(
      DpuBase + FixedPcdGet32(PcdDpuIntEnReg),
      FixedPcdGet32(PcdDpuIntDoneBit) |
      FixedPcdGet32(PcdDpuIntTeBit) | DPU_INT_ERROR_MASK);
}

STATIC VOID EFIAPI DpuInterruptHandler(
    IN HARDWARE_INTERRUPT_SOURCE Source,
    IN EFI_SYSTEM_CONTEXT        SystemContext)
{
  UINTN DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 IntSts = MmioRead32(DpuBase + FixedPcdGet32(PcdDpuIntStsReg));

  if ((IntSts & FixedPcdGet32(PcdDpuIntDoneBit)) != 0) {
    mDpuFrameInFlight = FALSE;
  }

  if (IntSts != 0) {
    MmioWrite32(DpuBase + FixedPcdGet32(PcdDpuIntClrReg), IntSts);
    MmioAnd32(
        DpuBase + FixedPcdGet32(PcdDpuIntEnReg),
        ~(IntSts & DPU_INT_ERROR_MASK));
  }

  mHardwareInterrupt->EndOfInterrupt(mHardwareInterrupt, Source);

  if (((IntSts & FixedPcdGet32(PcdDpuIntTeBit)) == 0) ||
      mDpuFrameInFlight) {
    return;
  }

  WriteBackInvalidateDataCacheRange(
      (void *)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->FrameBufferSize);
  EdpiTriggerFrame();
}

STATIC VOID EFIAPI OnExitBootServices(IN EFI_EVENT Event, IN VOID *Context)
{
  EFI_STATUS Status;
  EFI_TPL Tpl;
  UINTN DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 IntMask = FixedPcdGet32(PcdDpuIntDoneBit) |
      FixedPcdGet32(PcdDpuIntTeBit) | DPU_INT_ERROR_MASK;

  if (!mDpuInterruptRegistered) {
    return;
  }

  Tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
  MmioAnd32(DpuBase + FixedPcdGet32(PcdDpuIntEnReg), ~IntMask);
  Status = mHardwareInterrupt->DisableInterruptSource(
      mHardwareInterrupt, FixedPcdGet32(PcdDpuInterruptSource));
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: DPU interrupt disable failed: %r\n",
           Status));
    goto RestoreTpl;
  }

  Status = mHardwareInterrupt->RegisterInterruptSource(
      mHardwareInterrupt, FixedPcdGet32(PcdDpuInterruptSource), NULL);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: DPU interrupt unregister failed: %r\n",
           Status));
    goto RestoreTpl;
  }

  mDpuInterruptRegistered = FALSE;
  Status = EdpiWaitFrameDone();
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: DPU pending frame did not finish: %r\n",
           Status));
    goto RestoreTpl;
  }

  WriteBackInvalidateDataCacheRange(
      (VOID *)(UINTN)mDisplay.Mode->FrameBufferBase,
      mDisplay.Mode->FrameBufferSize);
  EdpiTriggerFrame();
  // The next owner uses the latched DONE bit as the idle handoff contract.
  Status = EdpiWaitFrameDone();
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: DPU final frame did not finish: %r\n",
           Status));
  }

RestoreTpl:
  gBS->RestoreTPL(Tpl);
}

STATIC
EFI_STATUS
EFIAPI
DisplayQueryMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info)
{
  EFI_STATUS Status;

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->AllocatePool(
      EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
      (VOID **)Info);

  ASSERT_EFI_ERROR(Status);

  *SizeOfInfo                   = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  (*Info)->Version              = This->Mode->Info->Version;
  (*Info)->HorizontalResolution = This->Mode->Info->HorizontalResolution;
  (*Info)->VerticalResolution   = This->Mode->Info->VerticalResolution;
  (*Info)->PixelFormat          = This->Mode->Info->PixelFormat;
  (*Info)->PixelsPerScanLine    = This->Mode->Info->PixelsPerScanLine;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
DisplaySetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber)
{
  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
DisplayBlt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *            BltBuffer,
    OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX,
    IN UINTN DestinationY, IN UINTN Width, IN UINTN Height,
    IN UINTN Delta OPTIONAL)
{

  RETURN_STATUS Status;
  EFI_TPL       Tpl;
  // Cursor timer callbacks must not interleave framebuffer writes.
  Tpl    = gBS->RaiseTPL(TPL_NOTIFY);
  Status = FrameBufferBlt(
      mFrameBufferBltLibConfigure, BltBuffer, BltOperation, SourceX, SourceY,
      DestinationX, DestinationY, Width, Height, Delta);
  if (!RETURN_ERROR(Status) && (BltOperation != EfiBltVideoToBltBuffer) &&
      (!FixedPcdGetBool(PcdDisplayCmdMode) ||
       FixedPcdGetBool(PcdDisplaySwitchToVideoMode))) {
    WriteBackInvalidateDataCacheRange(
        (void *)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->FrameBufferSize);
  }
  gBS->RestoreTPL(Tpl);

  return RETURN_ERROR(Status) ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SimpleFbDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{

  EFI_STATUS Status             = EFI_SUCCESS;
  EFI_HANDLE hUEFIDisplayHandle = NULL;

  DEBUG(
      (EFI_D_ERROR,
       "SimpleFbDxe: Retrieve MIPI FrameBuffer parameters from PCD\n"));
  UINT32 MipiFrameBufferAddr   = FixedPcdGet32(PcdMipiFrameBufferAddress);
  UINT32 MipiFrameBufferWidth  = FixedPcdGet32(PcdMipiFrameBufferWidth);
  UINT32 MipiFrameBufferHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);

  if (MipiFrameBufferAddr == 0 || MipiFrameBufferWidth == 0 ||
      MipiFrameBufferHeight == 0) {
    DEBUG((EFI_D_ERROR, "SimpleFbDxe: Invalid FrameBuffer parameters\n"));
    return EFI_DEVICE_ERROR;
  }

  if (mDisplay.Mode == NULL) {
    Status = gBS->AllocatePool(
        EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
        (VOID **)&mDisplay.Mode);

    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
      return Status;

    ZeroMem(mDisplay.Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
  }

  if (mDisplay.Mode->Info == NULL) {
    Status = gBS->AllocatePool(
        EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
        (VOID **)&mDisplay.Mode->Info);

    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
      return Status;

    ZeroMem(mDisplay.Mode->Info, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
  }

  mDisplay.Mode->MaxMode       = 1;
  mDisplay.Mode->Mode          = 0;
  mDisplay.Mode->Info->Version = 0;

  mDisplay.Mode->Info->HorizontalResolution = MipiFrameBufferWidth;
  mDisplay.Mode->Info->VerticalResolution   = MipiFrameBufferHeight;

  UINT32 LineLength =
      MipiFrameBufferWidth * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  UINT32               FrameBufferSize    = LineLength * MipiFrameBufferHeight;
  EFI_PHYSICAL_ADDRESS FrameBufferAddress = MipiFrameBufferAddr;

  mDisplay.Mode->Info->PixelsPerScanLine = MipiFrameBufferWidth;
  mDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
  mDisplay.Mode->SizeOfInfo      = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  mDisplay.Mode->FrameBufferBase = FrameBufferAddress;
  mDisplay.Mode->FrameBufferSize = FrameBufferSize;

  Status = FrameBufferBltConfigure(
      (VOID *)(UINTN)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->Info,
      mFrameBufferBltLibConfigure, &mFrameBufferBltLibConfigureSize);
  if (Status == RETURN_BUFFER_TOO_SMALL) {
    mFrameBufferBltLibConfigure = AllocatePool(mFrameBufferBltLibConfigureSize);
    if (mFrameBufferBltLibConfigure == NULL) {
      DEBUG((DEBUG_ERROR, "SimpleFbDxe: Failed to allocate Blt config\n"));
      return EFI_OUT_OF_RESOURCES;
    }
    Status = FrameBufferBltConfigure(
        (VOID *)(UINTN)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->Info,
        mFrameBufferBltLibConfigure, &mFrameBufferBltLibConfigureSize);
  }

  if (RETURN_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: FrameBufferBltConfigure failed: %r\n",
           Status));
    if (mFrameBufferBltLibConfigure != NULL) {
      FreePool(mFrameBufferBltLibConfigure);
      mFrameBufferBltLibConfigure = NULL;
    }
    return Status;
  }

  ZeroMem((void *)FrameBufferAddress, FrameBufferSize);
  WriteBackInvalidateDataCacheRange(
      (void *)FrameBufferAddress, FrameBufferSize);

  if (FixedPcdGetBool(PcdDisplayCmdMode) &&
      !FixedPcdGetBool(PcdDisplaySwitchToVideoMode)) {
    Status = gBS->LocateProtocol(
        &gHardwareInterruptProtocolGuid, NULL, (VOID **)&mHardwareInterrupt);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    EdpiDisableInterrupts();
    Status = mHardwareInterrupt->RegisterInterruptSource(
        mHardwareInterrupt, FixedPcdGet32(PcdDpuInterruptSource),
        DpuInterruptHandler);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    mDpuInterruptRegistered = TRUE;

    EdpiConfigureTe();
    EdpiTriggerFrame();

    Status = gBS->CreateEventEx(
        EVT_NOTIFY_SIGNAL, TPL_CALLBACK, OnExitBootServices, NULL,
        &gEfiEventExitBootServicesGuid, &mExitBootServicesEvent);
    if (EFI_ERROR(Status)) {
      EdpiDisableInterrupts();
      mHardwareInterrupt->RegisterInterruptSource(
          mHardwareInterrupt, FixedPcdGet32(PcdDpuInterruptSource), NULL);
      mDpuInterruptRegistered = FALSE;
      return Status;
    }
  }

  Status = gBS->InstallMultipleProtocolInterfaces(
      &hUEFIDisplayHandle, &gEfiDevicePathProtocolGuid, &mDisplayDevicePath,
      &gEfiGraphicsOutputProtocolGuid, &mDisplay, NULL);

  ASSERT_EFI_ERROR(Status);

  return Status;
}
