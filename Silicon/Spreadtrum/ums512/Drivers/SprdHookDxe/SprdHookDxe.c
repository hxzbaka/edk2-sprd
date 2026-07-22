#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Uefi/UefiSpec.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePath.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

STATIC EFI_EXIT_BOOT_SERVICES  mOldExitBootServices;
STATIC EFI_GET_MEMORY_MAP      mOldGetMemoryMap;

STATIC UINTN mGetMemoryMapCount = 0;
STATIC UINTN mExitBootCount     = 0;

STATIC UINTN LastMapKey         = 0;
STATIC UINTN LastMapSize        = 0;
STATIC UINTN LastDescSize       = 0;
STATIC UINT32 LastDescVersion   = 0;


STATIC EFI_ALLOCATE_PAGES mOldAllocatePages;
STATIC EFI_FREE_PAGES mOldFreePages;
STATIC EFI_SET_MEM mOldSetMem;
STATIC EFI_SET_VIRTUAL_ADDRESS_MAP mOldSetVirtualAddressMap;

STATIC
VOID
DumpMemoryMap (
    EFI_MEMORY_DESCRIPTOR *Map,
    UINTN Size,
    UINTN DescSize
    )
{
    UINTN Count = Size / DescSize;

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] MemoryMap Entries=%lu\n",
        Count
        ));

    for (UINTN i = 0; i < Count; i++)
    {
        EFI_MEMORY_DESCRIPTOR *Desc =
            (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + i * DescSize);

        DEBUG((
            EFI_D_ERROR,
            "[%03lu] Type=%02u  Phys=%016lx  Pages=%08lx  Attr=%016lx\n",
            i,
            Desc->Type,
            Desc->PhysicalStart,
            Desc->NumberOfPages,
            Desc->Attribute
            ));
    }
}


EFI_STATUS
EFIAPI
SprdHookSetVirtualAddressMap (
  UINTN MemoryMapSize,
  UINTN DescriptorSize,
  UINT32 DescriptorVersion,
  EFI_MEMORY_DESCRIPTOR *VirtualMap
  )
{
    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] SetVirtualAddressMap\n"
        "MapSize=%lx\n"
        "DescSize=%lx\n",
        MemoryMapSize,
        DescriptorSize
    ));

    return mOldSetVirtualAddressMap(
                MemoryMapSize,
                DescriptorSize,
                DescriptorVersion,
                VirtualMap
                );
}


VOID
EFIAPI
SprdHookSetMem (
  VOID *Buffer,
  UINTN Size,
  UINT8 Value
  )
{
  DEBUG((
    EFI_D_INFO,
    "[SPRDHOOK] SetMem %p %lx %02x\n",
    Buffer,
    Size,
    Value
    ));

  mOldSetMem(
      Buffer,
      Size,
      Value
      );
}

EFI_STATUS
EFIAPI
SprdHookFreePages (
  EFI_PHYSICAL_ADDRESS Memory,
  UINTN Pages
  )
{
  EFI_STATUS Status;

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] FreePages %lx %lu\n",
    Memory,
    Pages
    ));

  Status = mOldFreePages(
             Memory,
             Pages
             );

  return Status;
}

EFI_STATUS
EFIAPI
SprdHookAllocatePages (
  IN EFI_ALLOCATE_TYPE      Type,
  IN EFI_MEMORY_TYPE        MemoryType,
  IN UINTN                  Pages,
  IN OUT EFI_PHYSICAL_ADDRESS *Memory
  )
{
  EFI_STATUS Status;

  Status = mOldAllocatePages (
             Type,
             MemoryType,
             Pages,
             Memory
             );

  DEBUG((
    EFI_D_ERROR,
    "[SPRDHOOK] AllocatePages "
    "Type=%u "
    "MemType=%u "
    "Pages=%lu "
    "Addr=%lx "
    "%r\n",
    Type,
    MemoryType,
    Pages,
    Memory ? *Memory : 0,
    Status
    ));

  return Status;
}



EFI_STATUS
EFIAPI
SprdHookExitBootServices (
    EFI_HANDLE ImageHandle,
    UINTN MapKey
    )
{
    EFI_STATUS Status;

    mExitBootCount++;

    DEBUG((
        EFI_D_ERROR,
        "\r\n"
        "[SPRDHOOK] ==================================\r\n"
        "[SPRDHOOK] ExitBootServices #%u\r\n",
        mExitBootCount
        ));

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] Input MapKey : 0x%lx\r\n",
        MapKey
        ));

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] Last MapKey  : 0x%lx\r\n",
        LastMapKey
        ));

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] Last MapSize : 0x%lx\r\n",
        LastMapSize
        ));

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] Last DescSz  : 0x%lx\r\n",
        LastDescSize
        ));

    if (MapKey != LastMapKey)
    {
        DEBUG((
            EFI_D_ERROR,
            "[SPRDHOOK] WARNING: MapKey mismatch!\r\n"
            ));
    }

    Status = mOldExitBootServices(
                ImageHandle,
                MapKey
                );

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] ExitBootServices -> %r\r\n",
        Status
        ));

    if (Status == EFI_INVALID_PARAMETER)
    {
        DEBUG((
            EFI_D_ERROR,
            "[SPRDHOOK] INVALID_PARAMETER: Memory Map changed after GetMemoryMap!\r\n"
            ));
    }

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] ==================================\r\n"
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

    DEBUG((
        EFI_D_ERROR,
        "\r\n"
        "[SPRDHOOK] ==================================\r\n"
        "[SPRDHOOK] GetMemoryMap #%u\r\n"
        "[SPRDHOOK] Status=%r\r\n",
        mGetMemoryMapCount,
        Status
        ));

    if (MemoryMapSize != NULL)
    {
        DEBUG((
            EFI_D_ERROR,
            "[SPRDHOOK] MapSize = 0x%lx\r\n",
            *MemoryMapSize
            ));
    }

    if (!EFI_ERROR(Status))
    {
        DEBUG((
            EFI_D_ERROR,
            "[SPRDHOOK] MapKey  = 0x%lx\r\n",
            *MapKey
            ));

        DEBUG((
            EFI_D_ERROR,
            "[SPRDHOOK] DescSize = 0x%lx\r\n",
            *DescriptorSize
            ));

        DEBUG((
            EFI_D_ERROR,
            "[SPRDHOOK] DescVer  = %u\r\n",
            *DescriptorVersion
            ));

        if (LastMapKey != 0)
        {
            if (LastMapKey != *MapKey)
            {
                DEBUG((
                    EFI_D_ERROR,
                    "[SPRDHOOK] MapKey CHANGED "
                    "0x%lx -> 0x%lx\r\n",
                    LastMapKey,
                    *MapKey
                    ));
            }

            if (LastMapSize != *MemoryMapSize)
            {
                DEBUG((
                    EFI_D_ERROR,
                    "[SPRDHOOK] MapSize CHANGED "
                    "0x%lx -> 0x%lx\r\n",
                    LastMapSize,
                    *MemoryMapSize
                    ));
            }
        }

        LastMapKey       = *MapKey;
        LastMapSize      = *MemoryMapSize;
        LastDescSize     = *DescriptorSize;
        LastDescVersion  = *DescriptorVersion;
    }

    DEBUG((
        EFI_D_ERROR,
        "[SPRDHOOK] ==================================\r\n"
        ));

if (!EFI_ERROR(Status) &&
    MemoryMap != NULL)
{
    DumpMemoryMap(
        MemoryMap,
        *MemoryMapSize,
        *DescriptorSize
        );
}

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
    "[SPRDHOOK] SprdHookDxe Loaded\r\n"
    "[SPRDHOOK] BootServices=%p\r\n"
    "[SPRDHOOK] ExitBootServices=%p\r\n"
    "[SPRDHOOK] GetMemoryMap=%p\r\n"
    "[SPRDHOOK] ==================================\r\n"
    ));

  mOldExitBootServices = gBS->ExitBootServices;
  mOldGetMemoryMap     = gBS->GetMemoryMap;
  gBS->ExitBootServices = SprdHookExitBootServices;
  gBS->GetMemoryMap     = SprdHookGetMemoryMap;

mOldAllocatePages = gBS->AllocatePages;
gBS->AllocatePages = SprdHookAllocatePages;

mOldFreePages = gBS->FreePages;
gBS->FreePages = SprdHookFreePages;

mOldSetMem = gBS->SetMem;
gBS->SetMem = SprdHookSetMem;

mOldSetVirtualAddressMap = gST->RuntimeServices->SetVirtualAddressMap;

gST->RuntimeServices->SetVirtualAddressMap =
    SprdHookSetVirtualAddressMap;

  DEBUG ((
    EFI_D_ERROR,
    "[SPRDHOOK] Hooks Installed\r\n"
    ));

  return EFI_SUCCESS;
}