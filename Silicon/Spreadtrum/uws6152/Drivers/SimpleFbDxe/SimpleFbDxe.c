// SimpleFbDxe: Simple FrameBuffer
#include "Library/FrameBufferBltLib.h"
#include <Guid/EventGroup.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
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

STATIC UINT32 mPhysWidth;
STATIC UINT32 mPhysHeight;
STATIC UINT32 mPhysBpp;

STATIC UINT32 mGopWidth;
STATIC UINT32 mGopHeight;

STATIC EFI_EVENT                       mExitBootServicesEvent = NULL;
STATIC EFI_HARDWARE_INTERRUPT_PROTOCOL *mHardwareInterrupt;
STATIC BOOLEAN                         mDpuInterruptRegistered;

#define DPU_INT_FBC_PAYLOAD    BIT8
#define DPU_INT_FBC_HEADER     BIT9
#define DPU_INT_MMU_VAOR_READ  BIT16
#define DPU_INT_MMU_VAOR_WRITE BIT17
#define DPU_INT_MMU_INV_READ   BIT18
#define DPU_INT_MMU_INV_WRITE  BIT19
#define DPU_INT_ERROR_MASK                                                   \
  (DPU_INT_FBC_PAYLOAD | DPU_INT_FBC_HEADER | DPU_INT_MMU_VAOR_READ |       \
   DPU_INT_MMU_VAOR_WRITE | DPU_INT_MMU_INV_READ | DPU_INT_MMU_INV_WRITE)

STATIC VOID EdpiTriggerFrame(VOID)
{
  UINT32 DpuBase = FixedPcdGet32(PcdDpuBaseAddress);

  // BIT0 = RUN
  MmioOr32(DpuBase + FixedPcdGet32(PcdDpuCtrlReg), BIT0);
}

STATIC VOID EdpiDisableInterrupts(VOID)
{
  UINT32 DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 IntMask = FixedPcdGet32(PcdDpuIntDoneBit) |
                   FixedPcdGet32(PcdDpuIntTeBit) | DPU_INT_ERROR_MASK;

  MmioAnd32(
      DpuBase + FixedPcdGet32(PcdDpuIntEnReg),
      ~IntMask);
  MmioWrite32(DpuBase + FixedPcdGet32(PcdDpuIntClrReg), IntMask);
}

//
// Configure dpi_ctrl for external TE.
//
STATIC VOID EdpiConfigureTe(VOID)
{
  UINT32 DpuBase  = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 DpiCtrl  = FixedPcdGet32(PcdDpuDpiCtrlReg);
  UINT32 IntEnReg = FixedPcdGet32(PcdDpuIntEnReg);
  UINT32 DoneBit  = FixedPcdGet32(PcdDpuIntDoneBit);
  UINT32 TeBit    = FixedPcdGet32(PcdDpuIntTeBit);
  UINT32 TeBits;

  TeBits = FixedPcdGet32(PcdDpuDpiCtrlTeEnableBit) |
           FixedPcdGet32(PcdDpuDpiCtrlTeExternalBit);

  MmioOr32(DpuBase + DpiCtrl, TeBits);
  MmioOr32(DpuBase + IntEnReg, DoneBit | TeBit | DPU_INT_ERROR_MASK);
}

STATIC VOID EFIAPI DpuInterruptHandler(
    IN HARDWARE_INTERRUPT_SOURCE Source,
    IN EFI_SYSTEM_CONTEXT        SystemContext)
{
  UINT32 DpuBase   = FixedPcdGet32(PcdDpuBaseAddress);
  UINT32 IntStsReg = FixedPcdGet32(PcdDpuIntStsReg);
  UINT32 IntClrReg = FixedPcdGet32(PcdDpuIntClrReg);
  UINT32 TeBit     = FixedPcdGet32(PcdDpuIntTeBit);
  UINT32 BytesPerPixel;
  UINT32 IntSts;

  IntSts = MmioRead32(DpuBase + IntStsReg);
  if (IntSts) {
    MmioWrite32(DpuBase + IntClrReg, IntSts);
    MmioAnd32(
        DpuBase + FixedPcdGet32(PcdDpuIntEnReg),
        ~(IntSts & DPU_INT_ERROR_MASK));
  }

  mHardwareInterrupt->EndOfInterrupt(mHardwareInterrupt, Source);

  if ((IntSts & TeBit) == 0) {
    return;
  }

  BytesPerPixel = mPhysBpp / 8;
  WriteBackInvalidateDataCacheRange(
      (VOID *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress),
      (UINTN)mGopWidth * mGopHeight * BytesPerPixel);
  EdpiTriggerFrame();
}

// GOP protocol implementation
STATIC EFI_STATUS EFIAPI DisplayQueryMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info);

STATIC EFI_STATUS EFIAPI
DisplaySetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber);

STATIC EFI_STATUS EFIAPI DisplayBlt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL              *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL             *BltBuffer,
    OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX,
    IN UINTN DestinationY, IN UINTN Width, IN UINTN Height,
    IN UINTN Delta OPTIONAL);

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL mDisplay = {
    DisplayQueryMode, DisplaySetMode, DisplayBlt, NULL};

STATIC EFI_STATUS EFIAPI DisplayQueryMode(
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

STATIC EFI_STATUS EFIAPI
DisplaySetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber)
{
  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI DisplayBlt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL              *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL             *BltBuffer,
    OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX,
    IN UINTN DestinationY, IN UINTN Width, IN UINTN Height,
    IN UINTN Delta OPTIONAL)
{
  RETURN_STATUS Status;
  EFI_TPL       Tpl;

  Tpl    = gBS->RaiseTPL(TPL_NOTIFY);
  Status = FrameBufferBlt(
      mFrameBufferBltLibConfigure, BltBuffer, BltOperation, SourceX, SourceY,
      DestinationX, DestinationY, Width, Height, Delta);

  if (!RETURN_ERROR(Status) && BltOperation != EfiBltVideoToBltBuffer &&
      !FixedPcdGetBool(PcdDisplayCmdMode)) {
    WriteBackInvalidateDataCacheRange(
        (VOID *)(UINTN)mDisplay.Mode->FrameBufferBase,
        mDisplay.Mode->FrameBufferSize);
  }

  gBS->RestoreTPL(Tpl);

  return RETURN_ERROR(Status) ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
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

// Driver entry point
EFI_STATUS EFIAPI SimpleFbDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_HANDLE hUEFIDisplayHandle = NULL;
  UINT32     MipiFrameBufferAddr;
  UINT32     MipiFrameBufferWidth;
  UINT32     MipiFrameBufferHeight;

  MipiFrameBufferAddr   = FixedPcdGet32(PcdMipiFrameBufferAddress);
  MipiFrameBufferWidth  = FixedPcdGet32(PcdMipiFrameBufferWidth);
  MipiFrameBufferHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);

  if (MipiFrameBufferAddr == 0 || MipiFrameBufferWidth == 0 ||
      MipiFrameBufferHeight == 0) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: Invalid FrameBuffer parameters\n"));
    return EFI_DEVICE_ERROR;
  }

  mPhysWidth  = MipiFrameBufferWidth;
  mPhysHeight = MipiFrameBufferHeight;
  mPhysBpp    = FixedPcdGetBool(PcdReconfigureDisplay)
                    ? 32
                    : FixedPcdGet32(PcdMipiFrameBufferPixelBpp);

  if (mPhysBpp != 16 && mPhysBpp != 32) {
    DEBUG((DEBUG_ERROR, "SimpleFbDxe: Unsupported BPP %u\n", mPhysBpp));
    return EFI_DEVICE_ERROR;
  }

  DEBUG((DEBUG_INFO, "SimpleFbDxe: physical %ux%u %ubpp\n",
    mPhysWidth, mPhysHeight, mPhysBpp));

  // GOP mode structures
  if (mDisplay.Mode == NULL) {
    Status = gBS->AllocatePool(
        EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
        (VOID **)&mDisplay.Mode);
    ASSERT_EFI_ERROR(Status);
    ZeroMem(mDisplay.Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
  }

  if (mDisplay.Mode->Info == NULL) {
    Status = gBS->AllocatePool(
        EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
        (VOID **)&mDisplay.Mode->Info);
    ASSERT_EFI_ERROR(Status);
    ZeroMem(mDisplay.Mode->Info, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
  }

  mDisplay.Mode->MaxMode    = 1;
  mDisplay.Mode->Mode       = 0;
  mDisplay.Mode->SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  UINT32 DispW = mPhysWidth;
  UINT32 DispH = mPhysHeight;

  mGopWidth  = DispW;
  mGopHeight = DispH;

  mDisplay.Mode->Info->Version              = 0;
  mDisplay.Mode->Info->HorizontalResolution = DispW;
  mDisplay.Mode->Info->VerticalResolution   = DispH;

  UINT32 BytesPerPixel                   = mPhysBpp / 8;
  mDisplay.Mode->Info->PixelsPerScanLine = DispW;
  mDisplay.Mode->FrameBufferBase         = MipiFrameBufferAddr;
  mDisplay.Mode->FrameBufferSize =
      (UINTN)DispW * DispH * BytesPerPixel;

  if (mPhysBpp == 32) {
    mDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
    mDisplay.Mode->Info->PixelInformation.RedMask      = 0x00FF0000;
    mDisplay.Mode->Info->PixelInformation.GreenMask    = 0x0000FF00;
    mDisplay.Mode->Info->PixelInformation.BlueMask     = 0x000000FF;
    mDisplay.Mode->Info->PixelInformation.ReservedMask = 0xFF000000;
  }
  else {
    mDisplay.Mode->Info->PixelFormat                   = PixelBitMask;
    mDisplay.Mode->Info->PixelInformation.RedMask      = 0xF800;
    mDisplay.Mode->Info->PixelInformation.GreenMask    = 0x07E0;
    mDisplay.Mode->Info->PixelInformation.BlueMask     = 0x001F;
    mDisplay.Mode->Info->PixelInformation.ReservedMask = 0x0000;
  }

  // FrameBufferBltLib
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
    DEBUG(
        (DEBUG_ERROR, "SimpleFbDxe: FrameBufferBltConfigure failed: %r\n",
         Status));
    FreePool(mFrameBufferBltLibConfigure);
    mFrameBufferBltLibConfigure = NULL;
    return Status;
  }

  // Clear framebuffer
  {
    UINT8 *Pixels        = (UINT8 *)(UINTN)MipiFrameBufferAddr;
    UINT32 BytesPerPixel = mPhysBpp / 8;
    UINTN  Size          = (UINTN)mGopWidth * mGopHeight * BytesPerPixel;
    ZeroMem(Pixels, Size);
    WriteBackInvalidateDataCacheRange(Pixels, Size);
  }

  // Configure TE and send the initial frame.
  if (FixedPcdGetBool(PcdDisplayCmdMode)) {
    Status = gBS->LocateProtocol(
        &gHardwareInterruptProtocolGuid, NULL, (VOID **)&mHardwareInterrupt);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR,
             "SimpleFbDxe: Hardware interrupt protocol unavailable: %r\n",
             Status));
      return Status;
    }

    EdpiDisableInterrupts();
    Status = mHardwareInterrupt->RegisterInterruptSource(
        mHardwareInterrupt, FixedPcdGet32(PcdDpuInterruptSource),
        DpuInterruptHandler);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "SimpleFbDxe: DPU interrupt registration failed: %r\n",
             Status));
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
      DEBUG((DEBUG_ERROR,
             "SimpleFbDxe: ExitBootServices event creation failed: %r\n",
             Status));
      return Status;
    }
  }

  // Install GOP
  Status = gBS->InstallMultipleProtocolInterfaces(
      &hUEFIDisplayHandle, &gEfiDevicePathProtocolGuid, &mDisplayDevicePath,
      &gEfiGraphicsOutputProtocolGuid, &mDisplay, NULL);

  return Status;
}
