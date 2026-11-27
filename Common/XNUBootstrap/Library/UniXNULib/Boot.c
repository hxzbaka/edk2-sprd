// Copyright (c) 2026, UniXNU contributors. All rights reserved.
// SPDX-License-Identifier: BSD-2-Clause-Patent
#include "Internal.h"

EFI_STATUS
UniXNUBootEntry (EFI_HANDLE ImageHandle, UNIXNU_ENTRY *Entry)
{
  EFI_STATUS Status;
  EFI_DEVICE_PATH_PROTOCOL *Path;
  EFI_LOADED_IMAGE_PROTOCOL *Loaded;
  UNIXNU_LOADER_PROTOCOL *Loader;
  UNIXNU_LOADER_BOOT_REQUEST Request;
  EFI_HANDLE Image;
  EFI_HANDLE LoaderImage;
  VOID *Ramdisk;
  UINTN RamdiskSize;

  if (Entry->Type == UniXNUEntryAction) {
    if (Entry->Action == UniXNUActionReset) {
      gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
    if (Entry->Action == UniXNUActionShutdown) {
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
    return (Entry->Action == UniXNUActionExit) ? EFI_SUCCESS : EFI_UNSUPPORTED;
  }

  if (Entry->Type == UniXNUEntryXnu) {
    LoaderImage = NULL;
    if (Entry->Loader != NULL) {
      Path = UniXNUResolveImagePath (Entry->Volume, Entry->Loader);
      if (Path == NULL) {
        return EFI_NOT_FOUND;
      }
      Status = gBS->LoadImage (FALSE, ImageHandle, Path, NULL, 0, &LoaderImage);
      FreePool (Path);
      if (!EFI_ERROR (Status)) {
        Status = gBS->StartImage (LoaderImage, NULL, NULL);
        if (!EFI_ERROR (Status)) {
          Status = gBS->HandleProtocol (LoaderImage, &gUniXNULoaderProtocolGuid, (VOID **)&Loader);
        }
      }
    } else {
      Status = gBS->LocateProtocol (&gUniXNULoaderProtocolGuid, NULL, (VOID **)&Loader);
    }
    if (EFI_ERROR (Status)) {
      if (LoaderImage != NULL) {
        gBS->UnloadImage (LoaderImage);
      }
      return Status;
    }
    if (((Loader->Revision != UNIXNU_LOADER_REVISION_1) &&
         (Loader->Revision != UNIXNU_LOADER_REVISION)) || (Loader->Boot == NULL)) {
      if (LoaderImage != NULL) {
        gBS->UnloadImage (LoaderImage);
      }
      return EFI_UNSUPPORTED;
    }
    Ramdisk = NULL;
    RamdiskSize = 0;
    if (Entry->Initrd != NULL) {
      Status = UniXNUReadFile (Entry->Volume, Entry->Initrd, &Ramdisk, &RamdiskSize);
      if (EFI_ERROR (Status)) {
        if (LoaderImage != NULL) {
          gBS->UnloadImage (LoaderImage);
        }
        return Status;
      }
    }
    ZeroMem (&Request, sizeof (Request));
    Request.Volume = Entry->Volume;
    Request.KernelPath = Entry->Path;
    Request.BootArgs = Entry->Args;
    Request.Ramdisk = Ramdisk;
    Request.RamdiskSize = RamdiskSize;
    Status = Loader->Boot (Loader, &Request);
    if (Ramdisk != NULL) {
      FreePool (Ramdisk);
    }
    if (LoaderImage != NULL) {
      gBS->UnloadImage (LoaderImage);
    }
    return Status;
  }

  Path = UniXNUResolveImagePath (Entry->Volume, Entry->Path);
  if (Path == NULL) {
    return EFI_NOT_FOUND;
  }
  Image = NULL;
  Status = gBS->LoadImage (FALSE, ImageHandle, Path, NULL, 0, &Image);
  FreePool (Path);
  if (EFI_ERROR (Status)) {
    if ((Status == EFI_SECURITY_VIOLATION) && (Image != NULL)) {
      gBS->UnloadImage (Image);
    }
    return Status;
  }
  if (Entry->Args != NULL) {
    Status = gBS->HandleProtocol (Image, &gEfiLoadedImageProtocolGuid, (VOID **)&Loaded);
    if (EFI_ERROR (Status)) {
      gBS->UnloadImage (Image);
      return Status;
    }
    Loaded->LoadOptions = Entry->Args;
    Loaded->LoadOptionsSize = (UINT32)StrSize (Entry->Args);
  }
  return gBS->StartImage (Image, NULL, NULL);
}
