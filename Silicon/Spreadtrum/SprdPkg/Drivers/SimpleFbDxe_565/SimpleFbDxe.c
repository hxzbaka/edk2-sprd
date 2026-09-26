/* SimpleFbDxe: Simple FrameBuffer */
#include <PiDxe.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/BaseLib.h>
#include "Library/FrameBufferBltLib.h"
#include <Library/CacheMaintenanceLib.h>

/// Defines
/*
 * Convert enum video_log2_bpp to bytes and bits. Note we omit the outer
 * brackets to allow multiplication by fractional pixels.
 */
#define VNBYTES(bpix)	(1 << (bpix)) / 8
#define VNBITS(bpix)	(1 << (bpix))


/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
	VIDEO_BPP1	= 0,
	VIDEO_BPP2,
	VIDEO_BPP4,
	VIDEO_BPP8,
	VIDEO_BPP16,
	VIDEO_BPP32,
};

typedef struct {
  VENDOR_DEVICE_PATH DisplayDevicePath;
  EFI_DEVICE_PATH EndDevicePath;
} DISPLAY_DEVICE_PATH;

DISPLAY_DEVICE_PATH mDisplayDevicePath =
{
    {
      {
        HARDWARE_DEVICE_PATH,
        HW_VENDOR_DP,
        {
          (UINT8)(sizeof(VENDOR_DEVICE_PATH)),
          (UINT8)((sizeof(VENDOR_DEVICE_PATH)) >> 8),
        }
      },
      EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
    },
    {
      END_DEVICE_PATH_TYPE,
      END_ENTIRE_DEVICE_PATH_SUBTYPE,
      {
        sizeof(EFI_DEVICE_PATH_PROTOCOL),
        0
      }
    }
};

/// DeclaresSimpleFbDxe/SimpleFbDxe.c

STATIC FRAME_BUFFER_CONFIGURE        *mFrameBufferBltLibConfigure;
STATIC UINTN                         mFrameBufferBltLibConfigureSize;
STATIC EFI_PHYSICAL_ADDRESS          mScanoutFrameBufferBase;
STATIC UINTN                         mScanoutFrameBufferSize;
STATIC EFI_PHYSICAL_ADDRESS          mShadowFrameBufferBase;
STATIC UINTN                         mPixelsPerScanLine;

//
// GOP Mode->Info describes the REAL LK scanout (32bpp BGRX) so that an OS
// (e.g. Windows PE) writing directly to FrameBufferBase after ExitBootServices
// hits the actual scanned-out buffer.  UEFI-side Blt rendering keeps using the
// RGB565 shadow + a separate RGB565 mode info below.
//
STATIC EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  mBltInfoRgb565;

/*
 * LK leaves this panel scanning a 32-bit RGB framebuffer (logo_bpix=32),
 * while this driver deliberately exposes RGB565 to GOP clients.  Render GOP
 * into a 16-bit shadow buffer, then expand only the changed rectangle into
 * LK's existing 32-bit scanout.  No DPU/DSI register is touched.
 */
STATIC
VOID
SyncRgb565ToLkScanout (
  IN UINTN  X,
  IN UINTN  Y,
  IN UINTN  Width,
  IN UINTN  Height
  )
{
  UINT16  *Source;
  UINT32  *Destination;
  UINT16  Pixel;
  UINTN   Row;
  UINTN   Column;
  UINT32  Red;
  UINT32  Green;
  UINT32  Blue;

  if ((Width == 0) || (Height == 0)) {
    return;
  }

  for (Row = 0; Row < Height; Row++) {
    Source = (UINT16 *)(UINTN)mShadowFrameBufferBase +
             ((Y + Row) * mPixelsPerScanLine) + X;
    Destination = (UINT32 *)(UINTN)mScanoutFrameBufferBase +
                  ((Y + Row) * mPixelsPerScanLine) + X;
    for (Column = 0; Column < Width; Column++) {
      Pixel = Source[Column];
      Red   = (Pixel >> 11) & 0x1f;
      Green = (Pixel >> 5) & 0x3f;
      Blue  = Pixel & 0x1f;
      Destination[Column] = ((Red << 3) | (Red >> 2)) << 16 |
                            ((Green << 2) | (Green >> 4)) << 8 |
                            ((Blue << 3) | (Blue >> 2));
    }
  }

  WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)mScanoutFrameBufferBase, mScanoutFrameBufferSize);
}

STATIC
EFI_STATUS
EFIAPI
DisplayQueryMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
    IN  UINT32                                ModeNumber,
    OUT UINTN                                 *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
);

STATIC
EFI_STATUS
EFIAPI
DisplaySetMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN  UINT32                       ModeNumber
);

STATIC
EFI_STATUS
EFIAPI
DisplayBlt
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
    IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer,   OPTIONAL
    IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
    IN  UINTN                                   SourceX,
    IN  UINTN                                   SourceY,
    IN  UINTN                                   DestinationX,
    IN  UINTN                                   DestinationY,
    IN  UINTN                                   Width,
    IN  UINTN                                   Height,
    IN  UINTN                                   Delta         OPTIONAL
);

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL mDisplay = {
  DisplayQueryMode,
  DisplaySetMode,
  DisplayBlt,
  NULL
};

STATIC
EFI_STATUS
EFIAPI
DisplayQueryMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
    IN  UINT32                                ModeNumber,
    OUT UINTN                                 *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
)
{
    EFI_STATUS Status;
    Status = gBS->AllocatePool(
        EfiBootServicesData,
        sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
        (VOID **) Info);

    ASSERT_EFI_ERROR(Status);

    *SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    (*Info)->Version = This->Mode->Info->Version;
    (*Info)->HorizontalResolution = This->Mode->Info->HorizontalResolution;
    (*Info)->VerticalResolution = This->Mode->Info->VerticalResolution;
    (*Info)->PixelFormat = This->Mode->Info->PixelFormat;
    (*Info)->PixelInformation = This->Mode->Info->PixelInformation;
    (*Info)->PixelsPerScanLine = This->Mode->Info->PixelsPerScanLine;

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
DisplaySetMode
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN  UINT32                       ModeNumber
)
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
DisplayBlt
(
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL      *This,
    IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL     *BltBuffer,   OPTIONAL
    IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN  UINTN                             SourceX,
    IN  UINTN                             SourceY,
    IN  UINTN                             DestinationX,
    IN  UINTN                             DestinationY,
    IN  UINTN                             Width,
    IN  UINTN                             Height,
    IN  UINTN                             Delta         OPTIONAL
)
{

  RETURN_STATUS                         Status;
  EFI_TPL                               Tpl;
  //
  // We have to raise to TPL_NOTIFY, so we make an atomic write to the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  Tpl = gBS->RaiseTPL (TPL_NOTIFY);
  Status = FrameBufferBlt (
             mFrameBufferBltLibConfigure,
             BltBuffer,
             BltOperation,
             SourceX, SourceY,
             DestinationX, DestinationY, Width, Height,
             Delta
             );
  gBS->RestoreTPL (Tpl);

  if (!RETURN_ERROR (Status)) {
    switch (BltOperation) {
      case EfiBltVideoFill:
      case EfiBltBufferToVideo:
      case EfiBltVideoToVideo:
        SyncRgb565ToLkScanout (DestinationX, DestinationY, Width, Height);
        break;
      default:
        break;
    }
  }

  return RETURN_ERROR (Status) ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SimpleFbDxeInitialize
(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{

    EFI_STATUS          Status                  = EFI_SUCCESS;
    EFI_HANDLE          hUEFIDisplayHandle      = NULL;

    /* Retrieve simple frame buffer from pre-SEC bootloader */
    DEBUG((EFI_D_ERROR, "SimpleFbDxe: Retrieve MIPI FrameBuffer parameters from PCD\n"));
    UINT32              MipiFrameBufferAddr     = FixedPcdGet32(PcdMipiFrameBufferAddress);
    UINT32              MipiFrameBufferWidth    = FixedPcdGet32(PcdMipiFrameBufferWidth);
    UINT32              MipiFrameBufferHeight   = FixedPcdGet32(PcdMipiFrameBufferHeight);

    /* Sanity check */
    if (MipiFrameBufferAddr == 0 || MipiFrameBufferWidth == 0 || MipiFrameBufferHeight == 0)
    {
        DEBUG((EFI_D_ERROR, "SimpleFbDxe: Invalid FrameBuffer parameters\n"));
        return EFI_DEVICE_ERROR;
    }

    /* Prepare struct */
    if (mDisplay.Mode == NULL)
    {
        Status = gBS->AllocatePool(
            EfiBootServicesData,
            sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
            (VOID **) &mDisplay.Mode
        );

        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) return Status;

        ZeroMem(mDisplay.Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
    }
    
    if (mDisplay.Mode->Info == NULL)
    {
        Status = gBS->AllocatePool(
            EfiBootServicesData,
            sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
            (VOID **) &mDisplay.Mode->Info
        );

        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) return Status;

        ZeroMem(mDisplay.Mode->Info, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
    }

    /* Set information */
    mDisplay.Mode->MaxMode = 1;
    mDisplay.Mode->Mode = 0;
    mDisplay.Mode->Info->Version = 0;

    mDisplay.Mode->Info->HorizontalResolution = MipiFrameBufferWidth;
    mDisplay.Mode->Info->VerticalResolution = MipiFrameBufferHeight;

    /* GOP is RGB565, but LK scans the physical logo buffer as 32bpp RGB. */
    UINT32 LineLength = MipiFrameBufferWidth * VNBYTES(VIDEO_BPP16);
    UINT32 FrameBufferSize = LineLength * MipiFrameBufferHeight;
    EFI_PHYSICAL_ADDRESS FrameBufferAddress;

    mScanoutFrameBufferBase = MipiFrameBufferAddr;
    mScanoutFrameBufferSize = MipiFrameBufferWidth * MipiFrameBufferHeight * VNBYTES(VIDEO_BPP32);
    mPixelsPerScanLine = MipiFrameBufferWidth;
    FrameBufferAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocateZeroPool (FrameBufferSize);
    mShadowFrameBufferBase = FrameBufferAddress;
    if (FrameBufferAddress == 0) {
      return EFI_OUT_OF_RESOURCES;
    }

    mDisplay.Mode->Info->PixelsPerScanLine = MipiFrameBufferWidth;

    //
    // GOP exposes the REAL LK scanout (32bpp BGRX at MipiFrameBufferAddr).
    // SyncRgb565ToLkScanout writes B at byte0, G at byte1, R at byte2 ->
    // that is PixelBlueGreenRedReserved8BitPerColor in UEFI terms.
    // Windows PE writes this buffer directly after ExitBootServices, so the
    // framebuffer OS loaders see must be the actually scanned-out memory.
    //
    mDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
    mDisplay.Mode->SizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    mDisplay.Mode->FrameBufferBase = mScanoutFrameBufferBase;
    mDisplay.Mode->FrameBufferSize = mScanoutFrameBufferSize;

    //
    // UEFI-side Blt keeps the RGB565 shadow-buffer scheme: configure the
    // FrameBufferBltLib against the shadow buffer with a private RGB565 mode info.
    //
    mBltInfoRgb565.Version = 0;
    mBltInfoRgb565.HorizontalResolution = MipiFrameBufferWidth;
    mBltInfoRgb565.VerticalResolution = MipiFrameBufferHeight;
    mBltInfoRgb565.PixelsPerScanLine = MipiFrameBufferWidth;
    mBltInfoRgb565.PixelFormat = PixelBitMask;
    mBltInfoRgb565.PixelInformation.RedMask      = 0xF800;
    mBltInfoRgb565.PixelInformation.GreenMask    = 0x07E0;
    mBltInfoRgb565.PixelInformation.BlueMask     = 0x001F;
    mBltInfoRgb565.PixelInformation.ReservedMask = 0x0000;

    //
    // Create the FrameBufferBltLib configuration (RGB565 shadow buffer).
    //
    Status = FrameBufferBltConfigure (
                     (VOID *) (UINTN) mShadowFrameBufferBase,
                     &mBltInfoRgb565,
                     mFrameBufferBltLibConfigure,
                     &mFrameBufferBltLibConfigureSize
                     );
    if (Status == RETURN_BUFFER_TOO_SMALL) {
      mFrameBufferBltLibConfigure = AllocatePool (mFrameBufferBltLibConfigureSize);
      if (mFrameBufferBltLibConfigure != NULL) {
        Status = FrameBufferBltConfigure (
                         (VOID *) (UINTN) mShadowFrameBufferBase,
                         &mBltInfoRgb565,
                         mFrameBufferBltLibConfigure,
                         &mFrameBufferBltLibConfigureSize
                         );
      }
    }
    ASSERT_EFI_ERROR (Status);

    // Clear both the GOP RGB565 shadow and LK's physical 32-bit scanout.
    ZeroMem ((VOID *)(UINTN)FrameBufferAddress, FrameBufferSize);
    ZeroMem ((VOID *)(UINTN)mScanoutFrameBufferBase, mScanoutFrameBufferSize);
    WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)mScanoutFrameBufferBase, mScanoutFrameBufferSize);
 
    /* Register handle */
    Status = gBS->InstallMultipleProtocolInterfaces(
        &hUEFIDisplayHandle,
        &gEfiDevicePathProtocolGuid,
        &mDisplayDevicePath,
        &gEfiGraphicsOutputProtocolGuid,
        &mDisplay,
        NULL);

    ASSERT_EFI_ERROR (Status);

    return Status;

}