/** @file

  Copyright (c) 2026, hxzbaka. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __SPRD_BOOT_INIT_LIB_H__
#define __SPRD_BOOT_INIT_LIB_H__

#include <Uefi.h>

/**
  Parses a kernel command line string and stores key-value pairs.
  This function can be called once; subsequent calls will be ignored.

  @param[in] CmdLine  Null-terminated kernel command line string.
**/
VOID
EFIAPI
BootInit (
  IN CONST CHAR8 *CmdLine
  );

/**
  Retrieves the value of a boot argument by its key.

  @param[in]  Key       The key string (e.g., "androidboot.hardware").
  @param[out] Value     Buffer to receive the value (null-terminated).
  @param[in]  ValueSize Size of the Value buffer.

  @retval EFI_SUCCESS      Key found and value copied.
  @retval EFI_NOT_FOUND    Key not present.
  @retval EFI_BUFFER_TOO_SMALL Value too long for buffer.
  @retval EFI_NOT_READY    BootInit() has not been called yet.
**/
EFI_STATUS
EFIAPI
GetBootArg (
  IN  CONST CHAR8 *Key,
  OUT CHAR8       *Value,
  IN  UINTN       ValueSize
  );

/**
  Checks if a boolean boot argument exists (e.g., "rootwait").

  @param[in] Key   The key string.

  @retval TRUE   The key exists (as a flag without value).
  @retval FALSE  Otherwise, or if BootInit() not called.
**/
BOOLEAN
EFIAPI
HasBootArg (
  IN CONST CHAR8 *Key
  );

#endif