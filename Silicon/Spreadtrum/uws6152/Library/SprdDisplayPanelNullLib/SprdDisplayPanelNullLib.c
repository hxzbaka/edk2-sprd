/** @file
  Null implementation of Spreadtrum Display Panel Library.

  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/SprdDisplayPanelLib.h>

EFI_STATUS
EFIAPI
SprdDisplayPanelPrepare(IN CONST SPRD_DISPLAY_PANEL_IO *Io)
{
  (VOID)Io;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SprdDisplayPanelEnable(IN CONST SPRD_DISPLAY_PANEL_IO *Io)
{
  (VOID)Io;
  return EFI_SUCCESS;
}
