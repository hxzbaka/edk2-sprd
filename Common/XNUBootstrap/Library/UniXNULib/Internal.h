// Copyright (c) 2026, UniXNU contributors. All rights reserved.
// SPDX-License-Identifier: BSD-2-Clause-Patent
#ifndef UNIXNU_INTERNAL_H_
#define UNIXNU_INTERNAL_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UniXNULib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/UniXNULoader.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>

#define UNIXNU_VERSION_STRING "0.1.0"

typedef enum { TomlString, TomlInteger, TomlBoolean, TomlArray, TomlTable } TOML_TYPE;
typedef struct TOML_NODE TOML_NODE;
struct TOML_NODE {
  TOML_TYPE Type;
  CHAR8 *Key;
  UINTN Line;
  CHAR8 *String;
  INT64 Integer;
  BOOLEAN Boolean;
  BOOLEAN Defined;
  BOOLEAN TableArray;
  TOML_NODE *Child;
  TOML_NODE *LastChild;
  TOML_NODE *Next;
};

EFI_STATUS TomlParse (CONST CHAR8 *Text, UINTN Length, TOML_NODE **Root, CHAR8 *ErrorBuffer, UINTN ErrorBufferSize);
VOID TomlFree (TOML_NODE *Node);
TOML_NODE *TomlGet (TOML_NODE *Table, CONST CHAR8 *Key);
CONST CHAR8 *TomlGetString (TOML_NODE *Table, CONST CHAR8 *Key, CONST CHAR8 *Default);
INT64 TomlGetInteger (TOML_NODE *Table, CONST CHAR8 *Key, INT64 Default);
BOOLEAN TomlGetBoolean (TOML_NODE *Table, CONST CHAR8 *Key, BOOLEAN Default);
UINTN TomlCount (TOML_NODE *Array);

typedef enum { UniXNUEntryEfi, UniXNUEntryAction, UniXNUEntryXnu } UNIXNU_ENTRY_TYPE;
typedef enum { UniXNUActionNone, UniXNUActionReset, UniXNUActionShutdown, UniXNUActionExit } UNIXNU_ACTION;
typedef struct {
  CHAR8 *Id;
  CHAR16 *Title;
  UNIXNU_ENTRY_TYPE Type;
  UNIXNU_ACTION Action;
  EFI_HANDLE Volume;
  CHAR16 *Path;
  CHAR16 *Args;
  CHAR16 *Initrd;
  CHAR16 *Loader;
  CHAR16 *Confirm;
  CHAR16 **ConfirmTitles;
  CHAR16 **ConfirmArgs;
  UINTN ConfirmCount;
} UNIXNU_ENTRY;

typedef enum { UniXNUConsoleAuto, UniXNUConsoleGraphics, UniXNUConsoleText, UniXNUConsoleNone } UNIXNU_CONSOLE;
typedef enum { UniXNULayoutAuto, UniXNULayoutLandscape, UniXNULayoutPortrait } UNIXNU_LAYOUT;
#define UNIXNU_PORTRAIT_BELOW_DEFAULT 800
typedef struct {
  INTN Timeout;
  UINTN DefaultIndex;
  UINTN RotateSeconds;
  UINTN FallbackTimeout;
  UNIXNU_CONSOLE Console;
  CHAR16 *Title;
  UINTN Scale;
  BOOLEAN ShowLogo;
  UNIXNU_LAYOUT Layout;
  UINTN PortraitBelow;
  UNIXNU_ENTRY *Entries;
  UINTN EntryCount;
  EFI_HANDLE Volume;
  CHAR16 *SourceLabel;
  CHAR8 Error[160];
  CHAR8 Notice[96];
} UNIXNU_CONFIG;

VOID EFIAPI UniXNULog (CONST CHAR8 *Format, ...);
EFI_STATUS UniXNUConfigLoad (EFI_HANDLE ImageHandle, UNIXNU_CONFIG *Config);
VOID UniXNUConfigFree (UNIXNU_CONFIG *Config);
EFI_STATUS UniXNUReadFile (EFI_HANDLE Volume, CONST CHAR16 *Path, VOID **Buffer, UINTN *Size);
CHAR16 *UniXNUNormalizePath (CONST CHAR16 *Base OPTIONAL, CONST CHAR8 *Path);
CHAR16 *UniXNUAsciiToUnicode (CONST CHAR8 *String);
EFI_DEVICE_PATH_PROTOCOL *UniXNUResolveImagePath (EFI_HANDLE Volume, CONST CHAR16 *Path);
EFI_STATUS UniXNUBootEntry (EFI_HANDLE ImageHandle, UNIXNU_ENTRY *Entry);

#define UNIXNU_GLYPH_WIDTH 8
#define UNIXNU_GLYPH_HEIGHT 19
extern CONST UINT8 gCaFontGlyphs[95][UNIXNU_GLYPH_HEIGHT];
typedef enum { UniXNUUiGraphics, UniXNUUiText, UniXNUUiHeadless } UNIXNU_UI_MODE;
typedef struct {
  UNIXNU_UI_MODE Mode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  UINT32 Width;
  UINT32 Height;
  UINTN Scale;
  BOOLEAN Portrait;
  UINTN Margin;
  UINTN CardHeight;
  UINTN StatusLines;
  UINTN MenuTop;
  UINTN RowHeight;
  UINTN MenuLeft;
  UINTN MenuWidth;
  UINTN FooterTop;
  UINTN ContentTop;
  UINTN ContentBottom;
  UINTN FirstVisible;
  UINTN VisibleRows;
  UINTN TextColumns;
  UINTN TextRows;
  CHAR16 Info[96];
  CHAR16 Notice[96];
} UNIXNU_UI;

BOOLEAN UniXNUUiChoosePortrait (UNIXNU_LAYOUT Layout, UINTN PortraitBelow, UINT32 Width, UINT32 Height);
VOID UniXNUUiInit (UNIXNU_CONFIG *Config, UNIXNU_UI *Ui);
VOID UniXNUUiDrawFrame (UNIXNU_UI *Ui, UNIXNU_CONFIG *Config);
VOID UniXNUUiDrawEntry (UNIXNU_UI *Ui, UNIXNU_CONFIG *Config, UINTN Index, BOOLEAN Selected);
VOID UniXNUUiDrawStatus (UNIXNU_UI *Ui, UINTN Line, CONST CHAR16 *Text);
VOID UniXNULogoRender (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Buffer, UINTN Size, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Background);
UINTN UniXNUMenuRun (UNIXNU_UI *Ui, UNIXNU_CONFIG *Config, BOOLEAN Countdown);
BOOLEAN UniXNUMenuConfirm (UNIXNU_UI *Ui, UNIXNU_ENTRY *Entry, UINTN *Choice);
#endif
