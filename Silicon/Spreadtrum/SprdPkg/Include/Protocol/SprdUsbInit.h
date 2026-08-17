/** @file
  Private hand-off protocol between the UMS512 USB initialisation and MUSB
  host-controller DXE drivers.
**/

#ifndef SPRD_USB_INIT_PROTOCOL_H_
#define SPRD_USB_INIT_PROTOCOL_H_

#define SPRD_USB_INIT_PROTOCOL_GUID \
  { 0x889a1a07, 0xa8c4, 0x4f65, { 0xb9, 0x4b, 0x62, 0xf2, 0x5b, 0x1a, 0x9e, 0x11 } }

extern EFI_GUID  gSprdUsbInitProtocolGuid;

typedef struct _SPRD_USB_INIT_PROTOCOL SPRD_USB_INIT_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *SPRD_USB_SET_VBUS)(
  IN SPRD_USB_INIT_PROTOCOL  *This,
  IN BOOLEAN                 Enable
  );

struct _SPRD_USB_INIT_PROTOCOL {
  EFI_PHYSICAL_ADDRESS    MusbBase;
  UINTN                   MusbSize;
  SPRD_USB_SET_VBUS       SetVbus;
};

#endif
