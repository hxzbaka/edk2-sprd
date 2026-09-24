/** @file
  Spreadtrum ADI register access declarations.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
**/

#ifndef __SPRD_ADI_H__
#define __SPRD_ADI_H__

#include <Uefi.h>

UINT16
AdiRead (
  UINT16 Reg
  );

BOOLEAN
AdiReadChecked (
  UINT16 Reg,
  UINT16 *Value
  );

VOID
AdiWrite (
  UINT16 Reg,
  UINT16 Value
  );

VOID
AdiSetBits (
  UINT16 Reg,
  UINT16 Bits
  );

VOID
AdiClrBits (
  UINT16 Reg,
  UINT16 Bits
  );

#endif
