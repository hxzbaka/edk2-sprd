#ifndef SPRD_DISPLAY_PANEL_LIB_H_
#define SPRD_DISPLAY_PANEL_LIB_H_

#include <Uefi.h>

typedef
EFI_STATUS
(EFIAPI *SPRD_DISPLAY_DSI_WRITE)(
    IN CONST VOID  *Context,
    IN CONST UINT8 *Payload,
    IN UINTN        PayloadLength);

typedef struct {
  CONST VOID             *Context;
  SPRD_DISPLAY_DSI_WRITE Write;
} SPRD_DISPLAY_PANEL_IO;

EFI_STATUS
EFIAPI
SprdDisplayPanelPrepare(IN CONST SPRD_DISPLAY_PANEL_IO *Io);

EFI_STATUS
EFIAPI
SprdDisplayPanelEnable(IN CONST SPRD_DISPLAY_PANEL_IO *Io);

#endif
