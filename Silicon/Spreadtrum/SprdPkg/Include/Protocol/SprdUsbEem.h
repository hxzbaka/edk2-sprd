/** @file
  Transport protocol exported by SprdUsbDxe for an in-firmware KDNET client.
**/

#ifndef __SPRD_USB_EEM_PROTOCOL_H__
#define __SPRD_USB_EEM_PROTOCOL_H__

#include <Uefi.h>

#define SPRD_USB_EEM_PROTOCOL_GUID \
  { 0x0e8e2a0c, 0x86f2, 0x4bd2, { 0x8c, 0x3d, 0x81, 0x15, 0x9b, 0x13, 0x8b, 0x5a } }

typedef struct _SPRD_USB_EEM_PROTOCOL SPRD_USB_EEM_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *SPRD_USB_EEM_SEND_FRAME)(
  IN SPRD_USB_EEM_PROTOCOL  *This,
  IN CONST VOID             *Frame,
  IN UINTN                  FrameLength
  );

typedef
EFI_STATUS
(EFIAPI *SPRD_USB_EEM_RECEIVE_FRAME)(
  IN     SPRD_USB_EEM_PROTOCOL  *This,
  OUT    VOID                   *Frame,
  IN OUT UINTN                  *FrameLength
  );

typedef
BOOLEAN
(EFIAPI *SPRD_USB_EEM_LINK_UP)(
  IN SPRD_USB_EEM_PROTOCOL  *This
  );

struct _SPRD_USB_EEM_PROTOCOL {
  SPRD_USB_EEM_SEND_FRAME     SendFrame;
  SPRD_USB_EEM_RECEIVE_FRAME  ReceiveFrame;
  SPRD_USB_EEM_LINK_UP        LinkUp;
};

extern EFI_GUID  gSprdUsbEemProtocolGuid;

#endif
