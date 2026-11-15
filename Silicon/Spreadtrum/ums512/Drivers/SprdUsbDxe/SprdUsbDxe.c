/** @file
  SprdUsbDxe.c

  Requests normal UEFI USB bus binding after DXE USB drivers are present.
  This driver does not parse HID reports. Once MUSB publishes
  EFI_USB2_HC_PROTOCOL, UsbBusDxe performs device enumeration.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Guid/EventGroup.h>
#include <Protocol/Usb2HostController.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

STATIC
VOID
EFIAPI
SprdUsbConnectControllers (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_HANDLE  *Handles;
  EFI_STATUS  Status;
  UINTN       Count;
  UINTN       Index;

  Handles = NULL;
  Count = 0;
  if (!EFI_ERROR (gBS->LocateHandleBuffer (ByProtocol, &gEfiUsb2HcProtocolGuid,
                         NULL, &Count, &Handles))) {
    DEBUG ((DEBUG_INFO, "SPRD-USB: connecting %u USB host controller(s)\n", Count));
    for (Index = 0; Index < Count; Index++) {
      Status = gBS->ConnectController (Handles[Index], NULL, NULL, TRUE);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "SPRD-USB: USB bus controller connection failed: %r\n", Status));
      } else {
        DEBUG ((DEBUG_INFO, "SPRD-USB: USB bus controller connected successfully\n"));
      }
    }
    FreePool (Handles);
  }
  gBS->CloseEvent (Event);
}

EFI_STATUS
EFIAPI
SprdUsbDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  Status = gBS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
                  SprdUsbConnectControllers, NULL, &gEfiEventDxeDispatchGuid, &Event);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: DXE dispatch event creation failed: %r\n", Status));
  }
  return Status;
}