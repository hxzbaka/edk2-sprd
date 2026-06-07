#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Uefi/UefiSpec.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePath.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

STATIC EFI_EXIT_BOOT_SERVICES mOldExitBootServices;
STATIC EFI_GET_MEMORY_MAP     mOldGetMemoryMap;
STATIC EFI_IMAGE_LOAD      mOldLoadImage;
STATIC EFI_IMAGE_START     mOldStartImage;
STATIC EFI_ALLOCATE_POOL   mOldAllocatePool;
STATIC EFI_ALLOCATE_PAGES  mOldAllocatePages;

STATIC UINTN mExitBootServicesCount = 0;
STATIC UINTN mGetMemoryMapCount     = 0;

STATIC EFI_HANDLE_PROTOCOL mOldHandleProtocol;

STATIC
VOID
DumpLoadedImage (
  IN EFI_HANDLE ImageHandle
  )
{
  EFI_STATUS                Status;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
  CHAR16                    *Path;

  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                  );

  if (EFI_ERROR(Status)) {
    DEBUG((
      EFI_D_ERROR,
      "[SPRDHOOK] HandleProtocol LoadedImage failed %r\r\n",
      Status
      ));
    return;
  }

  Path = ConvertDevicePathToText(
           LoadedImage->FilePath,
           TRUE,
           TRUE
           );

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] ImageBase=%p Size=0x%lx\r\n",
    LoadedImage->ImageBase,
    LoadedImage->ImageSize
    ));

  if (Path != NULL) {
    DEBUG((
      EFI_D_ERROR,
      "[SPRDHOOK] File=%s\r\n",
      Path
      ));
    FreePool(Path);
  }
}

EFI_STATUS
EFIAPI
SprdHookLoadImage (
  BOOLEAN                   BootPolicy,
  EFI_HANDLE                ParentImageHandle,
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  VOID                      *SourceBuffer,
  UINTN                     SourceSize,
  EFI_HANDLE                *ImageHandle
  )
{
  EFI_STATUS Status;

if (DevicePath != NULL) {
  CHAR16 *PathText;

  PathText = ConvertDevicePathToText(
               DevicePath,
               TRUE,
               TRUE
               );

  if (PathText != NULL) {
    DEBUG((
      EFI_D_ERROR,
      "[SPRDHOOK] LoadImage %s\r\n",
      PathText
      ));

    FreePool(PathText);
  }
}

  Status = mOldLoadImage (
             BootPolicy,
             ParentImageHandle,
             DevicePath,
             SourceBuffer,
             SourceSize,
             ImageHandle
             );

  DEBUG ((
    EFI_D_ERROR,
    "[SPRDHOOK] LoadImage -> %r Handle=%p\r\n",
    Status,
    ImageHandle ? *ImageHandle : NULL
    ));

  return Status;
}


EFI_STATUS
EFIAPI
SprdHookExitBootServices (
  EFI_HANDLE ImageHandle,
  UINTN      MapKey
  )
{
  EFI_STATUS Status;

  DEBUG((
    EFI_D_ERROR,
    "\r\n"
    "[SPRDHOOK] ==================================\r\n"
    "[SPRDHOOK] ExitBootServices CALLED\r\n"
    "[SPRDHOOK] MapKey=0x%lx\r\n",
    MapKey
    ));

  DumpLoadedImage(ImageHandle);

  Status = mOldExitBootServices(
             ImageHandle,
             MapKey
             );

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] ExitBootServices Return=%r\r\n",
    Status
    ));

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] ==================================\r\n"
    ));

  return Status;
}

EFI_STATUS
EFIAPI
SprdHookStartImage (
  EFI_HANDLE ImageHandle,
  UINTN      *ExitDataSize,
  CHAR16     **ExitData
  )
{
  EFI_STATUS Status;

  DEBUG((
    EFI_D_ERROR,
    "\r\n"
    "[SPRDHOOK] ==================================\r\n"
    "[SPRDHOOK] StartImage\r\n"
    ));

  DumpLoadedImage(ImageHandle);

  Status = mOldStartImage (
             ImageHandle,
             ExitDataSize,
             ExitData
             );

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] StartImage Return=%r\r\n",
    Status
    ));

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] ==================================\r\n"
    ));

  return Status;
}

EFI_STATUS
EFIAPI
SprdHookAllocatePool (
  EFI_MEMORY_TYPE PoolType,
  UINTN           Size,
  VOID            **Buffer
  )
{
  EFI_STATUS Status;

  Status = mOldAllocatePool (
             PoolType,
             Size,
             Buffer
             );

  if (Size > 0x10000) {
    DEBUG ((
      EFI_D_ERROR,
      "[SPRDHOOK] AllocatePool Size=0x%lx -> %r Buffer=%p\r\n",
      Size,
      Status,
      Buffer ? *Buffer : NULL
      ));
  }

  return Status;
}

EFI_STATUS
EFIAPI
SprdHookAllocatePages (
  EFI_ALLOCATE_TYPE     Type,
  EFI_MEMORY_TYPE       MemoryType,
  UINTN                 Pages,
  EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  EFI_STATUS Status;

  Status = mOldAllocatePages (
             Type,
             MemoryType,
             Pages,
             Memory
             );

  DEBUG ((
    EFI_D_ERROR,
    "[SPRDHOOK] AllocatePages Pages=0x%lx -> %r Addr=0x%lx\r\n",
    Pages,
    Status,
    Memory ? *Memory : 0
    ));

  return Status;
}


EFI_STATUS
EFIAPI
SprdHookGetMemoryMap (
  IN OUT UINTN                 *MemoryMapSize,
  OUT EFI_MEMORY_DESCRIPTOR    *MemoryMap,
  OUT UINTN                    *MapKey,
  OUT UINTN                    *DescriptorSize,
  OUT UINT32                   *DescriptorVersion
  )
{
  EFI_STATUS Status;

  Status = mOldGetMemoryMap (
             MemoryMapSize,
             MemoryMap,
             MapKey,
             DescriptorSize,
             DescriptorVersion
             );

  mGetMemoryMapCount++;

  DEBUG ((
    EFI_D_ERROR,
    "[SPRDHOOK] GetMemoryMap #%u Size=0x%lx Status=%r\r\n",
    mGetMemoryMapCount,
    *MemoryMapSize,
    Status
    ));

  return Status;
}


EFI_STATUS
EFIAPI
SprdHookHandleProtocol (
  EFI_HANDLE Handle,
  EFI_GUID   *Protocol,
  VOID       **Interface
  )
{
  EFI_STATUS Status;

  Status = mOldHandleProtocol(
             Handle,
             Protocol,
             Interface
             );

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] HandleProtocol %g -> %r\r\n",
    Protocol,
    Status
    ));

  return Status;
}

EFI_STATUS
EFIAPI
SprdHookDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((
    EFI_D_ERROR,
    "\r\n"
    "[SPRDHOOK] ==================================\r\n"
    "[SPRDHOOK] Sprd1HookDxe Loaded\r\n"
    "[SPRDHOOK] BootServices=%p\r\n"
    "[SPRDHOOK] ExitBootServices=%p\r\n"
    "[SPRDHOOK] GetMemoryMap=%p\r\n"
    "[SPRDHOOK] ==================================\r\n"
    ));

  mOldExitBootServices = gBS->ExitBootServices;
  mOldGetMemoryMap     = gBS->GetMemoryMap;
mOldLoadImage     = gBS->LoadImage;
mOldStartImage    = gBS->StartImage;
mOldAllocatePool  = gBS->AllocatePool;
mOldAllocatePages = gBS->AllocatePages;
mOldHandleProtocol = gBS->HandleProtocol;

  gBS->ExitBootServices = SprdHookExitBootServices;
  gBS->GetMemoryMap     = SprdHookGetMemoryMap;
gBS->LoadImage     = SprdHookLoadImage;
gBS->StartImage    = SprdHookStartImage;
gBS->AllocatePool  = SprdHookAllocatePool;
gBS->AllocatePages = SprdHookAllocatePages;
gBS->HandleProtocol = SprdHookHandleProtocol;

  DEBUG ((
    EFI_D_ERROR,
    "[SPRDHOOK] Hooks Installed\r\n"
    ));

  return EFI_SUCCESS;
}