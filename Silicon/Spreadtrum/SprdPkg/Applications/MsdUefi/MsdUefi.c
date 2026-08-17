/** @file
  Transfers the Sprd raw eMMC from local UEFI use to the USB MSD function.
**/
#include <Uefi.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Pi/PiFirmwareFile.h>
#include <Pi/PiFirmwareVolume.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SprdUsbMsd.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

STATIC EFI_GUID  mFirmwareVolume2ProtocolGuid =
  { 0x220E73B6, 0x6BDB, 0x4413, { 0x84, 0x05, 0xB9, 0x74, 0xB1, 0x08, 0x61, 0x9A } };

STATIC EFI_GUID  mSprdEmmcDevicePathGuid =
  { 0x3EB4C147, 0x83B9, 0x0C39, { 0x04, 0x79, 0xEE, 0xB8, 0x0F, 0xC7, 0x52, 0x28 } };

STATIC EFI_GUID  mSimpleInitFileGuid =
  { 0x6D77B2BB, 0x69EB, 0x42AB, { 0xBE, 0xCF, 0x4F, 0x40, 0xC8, 0x95, 0x68, 0xC3 } };

STATIC
BOOLEAN
MsdUefiIsSprdEmmcChild (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  VENDOR_DEVICE_PATH  *VendorPath;

  if ((DevicePath == NULL) || (DevicePath->Type != HARDWARE_DEVICE_PATH) ||
      (DevicePath->SubType != HW_VENDOR_DP) ||
      (DevicePath->Length[0] != sizeof (VENDOR_DEVICE_PATH)) || (DevicePath->Length[1] != 0)) {
    return FALSE;
  }
  VendorPath = (VENDOR_DEVICE_PATH *)DevicePath;
  return CompareGuid (&VendorPath->Guid, &mSprdEmmcDevicePathGuid);
}

STATIC
EFI_STATUS
MsdUefiDisconnectEmmcFilesystems (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                *Handles;
  UINTN                     HandleCount;
  UINTN                     Index;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINTN                     Disconnected;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if (Status == EFI_NOT_FOUND) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Disconnected = 0;
  for (Index = 0; Index < HandleCount; Index++) {
    if (EFI_ERROR (gBS->HandleProtocol (Handles[Index], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath)) ||
        !MsdUefiIsSprdEmmcChild (DevicePath)) {
      continue;
    }
    Status = gBS->DisconnectController (Handles[Index], NULL, NULL);
    if (EFI_ERROR (Status)) {
      FreePool (Handles);
      return Status;
    }
    Disconnected++;
  }
  FreePool (Handles);
  DEBUG ((DEBUG_INFO, "SPRD MSD: disconnected %u eMMC filesystem handle(s)\n", Disconnected));
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MsdUefiFindRawEmmcHandle (
  OUT EFI_HANDLE  *RawEmmcHandle
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                *Handles;
  UINTN                     HandleCount;
  UINTN                     Index;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  if (RawEmmcHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < HandleCount; Index++) {
    if (EFI_ERROR (gBS->HandleProtocol (Handles[Index], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo)) ||
        EFI_ERROR (gBS->HandleProtocol (Handles[Index], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath)) ||
        (BlockIo->Media == NULL) || BlockIo->Media->LogicalPartition || !MsdUefiIsSprdEmmcChild (DevicePath)) {
      continue;
    }
    *RawEmmcHandle = Handles[Index];
    Status = EFI_SUCCESS;
    break;
  }
  FreePool (Handles);
  return Status;
}

STATIC
EFI_STATUS
MsdUefiRestoreEmmcFilesystem (
  IN EFI_HANDLE  RawEmmcHandle
  )
{
  if (RawEmmcHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  return gBS->ConnectController (RawEmmcHandle, NULL, NULL, TRUE);
}

STATIC
EFI_STATUS
MsdUefiStartSimpleInit (
  IN EFI_HANDLE  ParentImageHandle
  )
{
  EFI_STATUS                         Status;
  EFI_HANDLE                         *Handles;
  UINTN                              HandleCount;
  UINTN                              Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL      *FirmwareVolume;
  EFI_DEVICE_PATH_PROTOCOL           *FirmwareVolumePath;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  FvFileNode;
  EFI_DEVICE_PATH_PROTOCOL           *FilePath;
  EFI_HANDLE                         SimpleInitImage;
  VOID                               *FileBuffer;
  UINTN                              FileSize;
  EFI_FV_FILETYPE                    FileType;
  EFI_FV_FILE_ATTRIBUTES             Attributes;
  UINT32                             AuthenticationStatus;
  UINTN                              ExitDataSize;
  CHAR16                             *ExitData;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &mFirmwareVolume2ProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < HandleCount; Index++) {
    if (EFI_ERROR (gBS->HandleProtocol (Handles[Index], &mFirmwareVolume2ProtocolGuid, (VOID **)&FirmwareVolume)) ||
        EFI_ERROR (gBS->HandleProtocol (Handles[Index], &gEfiDevicePathProtocolGuid, (VOID **)&FirmwareVolumePath))) {
      continue;
    }

    FileBuffer = NULL;
    FileSize = 0;
    Status = FirmwareVolume->ReadFile (
                               FirmwareVolume,
                               &mSimpleInitFileGuid,
                               &FileBuffer,
                               &FileSize,
                               &FileType,
                               &Attributes,
                               &AuthenticationStatus
                               );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if (FileBuffer != NULL) {
      FreePool (FileBuffer);
    }

    ZeroMem (&FvFileNode, sizeof (FvFileNode));
    FvFileNode.Header.Type = MEDIA_DEVICE_PATH;
    FvFileNode.Header.SubType = MEDIA_PIWG_FW_FILE_DP;
    SetDevicePathNodeLength (&FvFileNode.Header, sizeof (FvFileNode));
    CopyGuid (&FvFileNode.FvFileName, &mSimpleInitFileGuid);
    FilePath = AppendDevicePathNode (FirmwareVolumePath, &FvFileNode.Header);
    if (FilePath == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      break;
    }

    SimpleInitImage = NULL;
    Status = gBS->LoadImage (FALSE, ParentImageHandle, FilePath, NULL, 0, &SimpleInitImage);
    FreePool (FilePath);
    if (EFI_ERROR (Status)) {
      break;
    }

    ExitData = NULL;
    ExitDataSize = 0;
    Status = gBS->StartImage (SimpleInitImage, &ExitDataSize, &ExitData);
    if (ExitData != NULL) {
      FreePool (ExitData);
    }
    break;
  }

  FreePool (Handles);
  return Status;
}

STATIC
VOID
MsdUefiWaitForEnter (
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          EventIndex;

  Print (L"Press Enter to stop USB MSD and restore UEFI eMMC mounts.\n");
  for (;;) {
    Status = gBS->WaitForEvent (1, &SystemTable->ConIn->WaitForKey, &EventIndex);
    if (EFI_ERROR (Status)) {
      return;
    }
    Status = SystemTable->ConIn->ReadKeyStroke (SystemTable->ConIn, &Key);
    if (!EFI_ERROR (Status) && (Key.UnicodeChar == CHAR_CARRIAGE_RETURN)) {
      return;
    }
  }
}

EFI_STATUS
EFIAPI
MsdUefiEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  EFI_STATUS             RestoreStatus;
  EFI_HANDLE             RawEmmcHandle;
  SPRD_USB_MSD_PROTOCOL  *Msd;

  if (!FeaturePcdGet (UsbMassStorage)) {
    Print (L"Platform does not support USB Mass Storage. Exiting in 3 seconds.\n");
    MicroSecondDelay (3 * 1000 * 1000);
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (&gSprdUsbMsdProtocolGuid, NULL, (VOID **)&Msd);
  if (EFI_ERROR (Status)) {
    Print (L"USB MSD backend is not loaded: %r\n", Status);
    return Status;
  }
  Status = MsdUefiFindRawEmmcHandle (&RawEmmcHandle);
  if (EFI_ERROR (Status)) {
    Print (L"Raw eMMC was not found: %r\n", Status);
    return Status;
  }
  Status = MsdUefiDisconnectEmmcFilesystems ();
  if (EFI_ERROR (Status)) {
    Print (L"eMMC filesystem disconnect failed: %r\n", Status);
    return Status;
  }
  Status = Msd->Start (Msd);
  if (EFI_ERROR (Status)) {
    Print (L"USB MSD start failed: %r\n", Status);
    MsdUefiRestoreEmmcFilesystem (RawEmmcHandle);
    return Status;
  }
  Print (L"USB MSD owns raw eMMC. USB was re-enumerated.\n");
  MsdUefiWaitForEnter (SystemTable);

  Status = Msd->Stop (Msd);
  if (EFI_ERROR (Status)) {
    Print (L"USB MSD stop failed: %r\n", Status);
  }
  RestoreStatus = MsdUefiRestoreEmmcFilesystem (RawEmmcHandle);
  if (EFI_ERROR (RestoreStatus)) {
    Print (L"eMMC filesystem restore failed: %r\n", RestoreStatus);
    return RestoreStatus;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Print (L"UEFI eMMC mounts restored. Starting SimpleInit GUI.\n");
  Status = MsdUefiStartSimpleInit (ImageHandle);
  if (EFI_ERROR (Status)) {
    Print (L"SimpleInit GUI start failed: %r\n", Status);
  }
  return Status;
}
