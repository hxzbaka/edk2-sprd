/* SimpleFbDxe: Simple FrameBuffer */
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

/// Defines
/*
 * Convert enum video_log2_bpp to bytes and bits. Note we omit the outer
 * brackets to allow multiplication by fractional pixels.
 */
#define VNBYTES(bpix) (1 << (bpix)) / 8
#define VNBITS(bpix) (1 << (bpix))

#define FB_BITS_PER_PIXEL (32)
#define FB_BYTES_PER_PIXEL (FB_BITS_PER_PIXEL / 8)

/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
  VIDEO_BPP1 = 0,
  VIDEO_BPP2,
  VIDEO_BPP4,
  VIDEO_BPP8,
  VIDEO_BPP16,
  VIDEO_BPP32,
};

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

/// Declares

STATIC FRAME_BUFFER_CONFIGURE *mFrameBufferBltLibConfigure;
STATIC UINTN                   mFrameBufferBltLibConfigureSize;
STATIC EFI_EVENT                       mExitBootServicesEvent;
STATIC EFI_HARDWARE_INTERRUPT_PROTOCOL *mHardwareInterrupt;
STATIC BOOLEAN                         mDpuInterruptRegistered;

#define DPU_INT_ERROR_MASK BIT2

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

  MmioOr32(DpuBase + FixedPcdGet32(PcdDpuCtrlReg), BIT0);
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

  if (IntSts != 0) {
    MmioWrite32(DpuBase + FixedPcdGet32(PcdDpuIntClrReg), IntSts);
    MmioAnd32(
        DpuBase + FixedPcdGet32(PcdDpuIntEnReg),
        ~(IntSts & DPU_INT_ERROR_MASK));
  }

  mHardwareInterrupt->EndOfInterrupt(mHardwareInterrupt, Source);

  if ((IntSts & FixedPcdGet32(PcdDpuIntTeBit)) == 0) {
    return;
  }

  WriteBackInvalidateDataCacheRange(
      (void *)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->FrameBufferSize);
  EdpiTriggerFrame();
}

STATIC VOID EFIAPI OnExitBootServices(IN EFI_EVENT Event, IN VOID *Context)
{
  EFI_STATUS Status;

  if (!mDpuInterruptRegistered) {
    return;
  }

  EdpiDisableInterrupts();
  Status = mHardwareInterrupt->RegisterInterruptSource(
      mHardwareInterrupt, FixedPcdGet32(PcdDpuInterruptSource), NULL);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: DPU interrupt unregister failed: %r\n",
           Status));
  }

  mDpuInterruptRegistered = FALSE;
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
  //
  // We have to raise to TPL_NOTIFY, so we make an atomic write to the frame
  // buffer. We would not want a timer based event (Cursor, ...) to come in
  // while we are doing this operation.
  //
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

  /* Retrieve simple frame buffer from pre-SEC bootloader */
  DEBUG(
      (EFI_D_ERROR,
       "SimpleFbDxe: Retrieve MIPI FrameBuffer parameters from PCD\n"));
  UINT32 MipiFrameBufferAddr   = FixedPcdGet32(PcdMipiFrameBufferAddress);
  UINT32 MipiFrameBufferWidth  = FixedPcdGet32(PcdMipiFrameBufferWidth);
  UINT32 MipiFrameBufferHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);

  /* Sanity check */
  if (MipiFrameBufferAddr == 0 || MipiFrameBufferWidth == 0 ||
      MipiFrameBufferHeight == 0) {
    DEBUG((EFI_D_ERROR, "SimpleFbDxe: Invalid FrameBuffer parameters\n"));
    return EFI_DEVICE_ERROR;
  }

  /* Prepare struct */
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

  /* Set information */
  mDisplay.Mode->MaxMode       = 1;
  mDisplay.Mode->Mode          = 0;
  mDisplay.Mode->Info->Version = 0;

  mDisplay.Mode->Info->HorizontalResolution = MipiFrameBufferWidth;
  mDisplay.Mode->Info->VerticalResolution   = MipiFrameBufferHeight;

  /* SimpleFB runs on a8r8g8b8 (VIDEO_BPP32) for DB410c */
  UINT32               LineLength = MipiFrameBufferWidth * VNBYTES(VIDEO_BPP32);
  UINT32               FrameBufferSize    = LineLength * MipiFrameBufferHeight;
  EFI_PHYSICAL_ADDRESS FrameBufferAddress = MipiFrameBufferAddr;

  mDisplay.Mode->Info->PixelsPerScanLine = MipiFrameBufferWidth;
  mDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
  mDisplay.Mode->SizeOfInfo      = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  mDisplay.Mode->FrameBufferBase = FrameBufferAddress;
  mDisplay.Mode->FrameBufferSize = FrameBufferSize;

  //
  // Create the FrameBufferBltLib configuration.
  //
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

  // zhuowei: clear the screen to black
  // UEFI standard requires this, since text is white - see
  // OvmfPkg/QemuVideoDxe/Gop.c
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

  /* Register handle */
  Status = gBS->InstallMultipleProtocolInterfaces(
      &hUEFIDisplayHandle, &gEfiDevicePathProtocolGuid, &mDisplayDevicePath,
      &gEfiGraphicsOutputProtocolGuid, &mDisplay, NULL);

  ASSERT_EFI_ERROR(Status);

  return Status;
}
