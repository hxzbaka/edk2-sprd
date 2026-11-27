// Copyright (c) 2026, UniXNU contributors. All rights reserved.
// SPDX-License-Identifier: BSD-2-Clause-Patent
#include "Internal.h"

STATIC CONST CHAR8 mFallback[] =
  "version = 1\n[boot]\ntimeout = -1\n[ui]\ntitle = \"UniXNU\"\n"
  "[[entry]]\nid = \"exit\"\ntitle = \"Return to firmware\"\ntype = \"action\"\naction = \"exit\"\n";

VOID
UniXNUConfigFree (UNIXNU_CONFIG *Config)
{
  UINTN Index;
  UINTN Choice;
  UNIXNU_ENTRY *Entry;

  for (Index = 0; Index < Config->EntryCount; Index++) {
    Entry = &Config->Entries[Index];
    if (Entry->Id != NULL) FreePool (Entry->Id);
    if (Entry->Title != NULL) FreePool (Entry->Title);
    if (Entry->Path != NULL) FreePool (Entry->Path);
    if (Entry->Args != NULL) FreePool (Entry->Args);
    if (Entry->Initrd != NULL) FreePool (Entry->Initrd);
    if (Entry->Loader != NULL) FreePool (Entry->Loader);
    if (Entry->Confirm != NULL) FreePool (Entry->Confirm);
    for (Choice = 0; Choice < Entry->ConfirmCount; Choice++) {
      if ((Entry->ConfirmTitles != NULL) && (Entry->ConfirmTitles[Choice] != NULL)) FreePool (Entry->ConfirmTitles[Choice]);
      if ((Entry->ConfirmArgs != NULL) && (Entry->ConfirmArgs[Choice] != NULL)) FreePool (Entry->ConfirmArgs[Choice]);
    }
    if (Entry->ConfirmTitles != NULL) FreePool (Entry->ConfirmTitles);
    if (Entry->ConfirmArgs != NULL) FreePool (Entry->ConfirmArgs);
  }
  if (Config->Entries != NULL) FreePool (Config->Entries);
  if (Config->Title != NULL) FreePool (Config->Title);
  if (Config->SourceLabel != NULL) FreePool (Config->SourceLabel);
  ZeroMem (Config, sizeof (*Config));
}

STATIC EFI_STATUS
ParseConfig (TOML_NODE *Root, CONST CHAR16 *Base, UNIXNU_CONFIG *Config)
{
  TOML_NODE *Boot;
  TOML_NODE *Ui;
  TOML_NODE *List;
  TOML_NODE *Item;
  TOML_NODE *Default;
  TOML_NODE *Titles;
  TOML_NODE *Arguments;
  TOML_NODE *Title;
  TOML_NODE *Argument;
  TOML_NODE *Loader;
  UNIXNU_ENTRY *Entry;
  CONST CHAR8 *Value;
  CONST CHAR8 *Path;
  UINTN Index;
  UINTN Choice;
  INT64 Number;

  if ((TomlGetInteger (Root, "version", 0) != 1) ||
      (TomlGet (Root, "driver") != NULL) || (TomlGet (Root, "dt_patch") != NULL) ||
      (TomlGet (Root, "block_slice") != NULL) || (TomlGet (Root, "acpi") != NULL) ||
      TomlGetBoolean (TomlGet (Root, "kits"), "autoload", FALSE)) {
    return EFI_UNSUPPORTED;
  }
  Boot = TomlGet (Root, "boot");
  Number = TomlGetInteger (Boot, "timeout", 5);
  if ((Number < -1) || (Number > 86400)) return EFI_INVALID_PARAMETER;
  Config->Timeout = (INTN)Number;
  Number = TomlGetInteger (Boot, "rotate", 0);
  if ((Number < 0) || (Number > 86400)) return EFI_INVALID_PARAMETER;
  Config->RotateSeconds = (UINTN)Number;
  Number = TomlGetInteger (Boot, "fallback_timeout", 10);
  if ((Number < 1) || (Number > 86400)) return EFI_INVALID_PARAMETER;
  Config->FallbackTimeout = (UINTN)Number;

  Ui = TomlGet (Root, "ui");
  Config->Title = UniXNUAsciiToUnicode (TomlGetString (Ui, "title", "UniXNU"));
  Config->ShowLogo = TomlGetBoolean (Ui, "logo", TRUE);
  Number = TomlGetInteger (Ui, "scale", 0);
  if ((Number < 0) || (Number > 16)) return EFI_INVALID_PARAMETER;
  Config->Scale = (UINTN)Number;
  Number = TomlGetInteger (Ui, "portrait_below", UNIXNU_PORTRAIT_BELOW_DEFAULT);
  if ((Number < 1) || (Number > 10000)) return EFI_INVALID_PARAMETER;
  Config->PortraitBelow = (UINTN)Number;
  Value = TomlGetString (Ui, "layout", "auto");
  if (AsciiStrCmp (Value, "auto") == 0) Config->Layout = UniXNULayoutAuto;
  else if (AsciiStrCmp (Value, "portrait") == 0) Config->Layout = UniXNULayoutPortrait;
  else if (AsciiStrCmp (Value, "landscape") == 0) Config->Layout = UniXNULayoutLandscape;
  else return EFI_INVALID_PARAMETER;
  Value = TomlGetString (Ui, "console", "auto");
  if (AsciiStrCmp (Value, "auto") == 0) Config->Console = UniXNUConsoleAuto;
  else if (AsciiStrCmp (Value, "graphics") == 0) Config->Console = UniXNUConsoleGraphics;
  else if (AsciiStrCmp (Value, "text") == 0) Config->Console = UniXNUConsoleText;
  else if (AsciiStrCmp (Value, "none") == 0) Config->Console = UniXNUConsoleNone;
  else return EFI_INVALID_PARAMETER;
  if (Config->Title == NULL) return EFI_OUT_OF_RESOURCES;

  List = TomlGet (Root, "entry");
  if ((List == NULL) || (List->Type != TomlArray) || !List->TableArray || (TomlCount (List) == 0) || (TomlCount (List) > 32)) return EFI_INVALID_PARAMETER;
  Config->Entries = AllocateZeroPool (TomlCount (List) * sizeof (*Config->Entries));
  if (Config->Entries == NULL) return EFI_OUT_OF_RESOURCES;
  for (Item = List->Child; Item != NULL; Item = Item->Next) {
    if (!TomlGetBoolean (Item, "enabled", TRUE)) continue;
    Entry = &Config->Entries[Config->EntryCount++];
    Value = TomlGetString (Item, "id", NULL);
    if (Value == NULL) return EFI_INVALID_PARAMETER;
    Entry->Id = AllocateCopyPool (AsciiStrSize (Value), Value);
    Entry->Title = UniXNUAsciiToUnicode (TomlGetString (Item, "title", Value));
    Entry->Volume = Config->Volume;
    if ((Entry->Id == NULL) || (Entry->Title == NULL)) return EFI_OUT_OF_RESOURCES;
    Value = TomlGetString (Item, "type", "efi");
    if (AsciiStrCmp (Value, "efi") == 0) Entry->Type = UniXNUEntryEfi;
    else if (AsciiStrCmp (Value, "xnu") == 0) Entry->Type = UniXNUEntryXnu;
    else if (AsciiStrCmp (Value, "action") == 0) Entry->Type = UniXNUEntryAction;
    else return EFI_UNSUPPORTED;
    if ((TomlGet (Item, "dtb") != NULL) || (TomlGet (Item, "dt_patch") != NULL) ||
        (TomlGet (Item, "chosen") != NULL) || (TomlGet (Item, "kext") != NULL) ||
        (TomlGet (Item, "variable") != NULL)) return EFI_UNSUPPORTED;
    if (Entry->Type == UniXNUEntryAction) {
      Value = TomlGetString (Item, "action", "exit");
      if (AsciiStrCmp (Value, "exit") == 0) Entry->Action = UniXNUActionExit;
      else if (AsciiStrCmp (Value, "reset") == 0) Entry->Action = UniXNUActionReset;
      else if (AsciiStrCmp (Value, "shutdown") == 0) Entry->Action = UniXNUActionShutdown;
      else return EFI_UNSUPPORTED;
    } else {
      Path = TomlGetString (Item, "path", NULL);
      if (Path == NULL) return EFI_INVALID_PARAMETER;
      Entry->Path = UniXNUNormalizePath ((AsciiStrnCmp (Path, "fv:", 3) == 0) ? NULL : Base, Path);
      if (Entry->Path == NULL) return EFI_OUT_OF_RESOURCES;
    }
    Value = TomlGetString (Item, "args", NULL);
    if (Value != NULL && (Entry->Args = UniXNUAsciiToUnicode (Value)) == NULL) return EFI_OUT_OF_RESOURCES;
    Value = TomlGetString (Item, "initrd", NULL);
    if (Value != NULL) {
      if (Entry->Type != UniXNUEntryXnu) return EFI_UNSUPPORTED;
      Entry->Initrd = UniXNUNormalizePath (Base, Value);
      if (Entry->Initrd == NULL) return EFI_OUT_OF_RESOURCES;
    }
    Loader = TomlGet (Item, "loader");
    if (Loader != NULL) {
      if (Entry->Type != UniXNUEntryXnu) return EFI_UNSUPPORTED;
      if ((Loader->Type != TomlString) || (Loader->String[0] == '\0')) return EFI_INVALID_PARAMETER;
      Entry->Loader = UniXNUNormalizePath (Base, Loader->String);
      if (Entry->Loader == NULL) return EFI_OUT_OF_RESOURCES;
    }
    Value = TomlGetString (Item, "confirm", NULL);
    if (Value != NULL && (Entry->Confirm = UniXNUAsciiToUnicode (Value)) == NULL) return EFI_OUT_OF_RESOURCES;
    Titles = TomlGet (Item, "confirm_titles");
    Arguments = TomlGet (Item, "confirm_args");
    if ((Titles != NULL) || (Arguments != NULL)) {
      if ((Entry->Confirm == NULL) || (Titles == NULL) || (Arguments == NULL) ||
          (Titles->Type != TomlArray) || (Arguments->Type != TomlArray) ||
          (TomlCount (Titles) == 0) || (TomlCount (Titles) != TomlCount (Arguments))) return EFI_INVALID_PARAMETER;
      Entry->ConfirmCount = TomlCount (Titles);
      Entry->ConfirmTitles = AllocateZeroPool (Entry->ConfirmCount * sizeof (CHAR16 *));
      Entry->ConfirmArgs = AllocateZeroPool (Entry->ConfirmCount * sizeof (CHAR16 *));
      if ((Entry->ConfirmTitles == NULL) || (Entry->ConfirmArgs == NULL)) return EFI_OUT_OF_RESOURCES;
      for (Choice = 0, Title = Titles->Child, Argument = Arguments->Child; Choice < Entry->ConfirmCount; Choice++, Title = Title->Next, Argument = Argument->Next) {
        if ((Title->Type != TomlString) || (Argument->Type != TomlString)) return EFI_INVALID_PARAMETER;
        Entry->ConfirmTitles[Choice] = UniXNUAsciiToUnicode (Title->String);
        Entry->ConfirmArgs[Choice] = UniXNUAsciiToUnicode (Argument->String);
        if ((Entry->ConfirmTitles[Choice] == NULL) || (Entry->ConfirmArgs[Choice] == NULL)) return EFI_OUT_OF_RESOURCES;
      }
    }
  }
  if (Config->EntryCount == 0) return EFI_NOT_FOUND;
  Default = TomlGet (Boot, "default");
  if ((Default != NULL) && (Default->Type == TomlString)) {
    for (Index = 0; Index < Config->EntryCount; Index++) {
      if (AsciiStrCmp (Config->Entries[Index].Id, Default->String) == 0) break;
    }
    if (Index == Config->EntryCount) return EFI_NOT_FOUND;
    Config->DefaultIndex = Index;
  } else if ((Default != NULL) && (Default->Type == TomlInteger)) {
    if ((Default->Integer < 0) || ((UINT64)Default->Integer >= Config->EntryCount)) return EFI_INVALID_PARAMETER;
    Config->DefaultIndex = (UINTN)Default->Integer;
  }
  if (Config->Entries[Config->DefaultIndex].Confirm != NULL) {
    for (Index = 0; Index < Config->EntryCount; Index++) {
      if (Config->Entries[Index].Confirm == NULL) break;
    }
    if (Index == Config->EntryCount) Config->Timeout = -1;
    else Config->DefaultIndex = Index;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
UniXNUConfigLoad (EFI_HANDLE ImageHandle, UNIXNU_CONFIG *Config)
{
  EFI_STATUS Status;
  STATIC CONST CHAR16 OptionsPrefix[] = L"unixnu:config=";
  EFI_LOADED_IMAGE_PROTOCOL *Loaded;
  CONST CHAR16 *ConfigPath;
  BOOLEAN ExplicitConfig;
  EFI_HANDLE *Handles;
  UINTN Count;
  UINTN Index;
  VOID *Text;
  UINTN Size;
  TOML_NODE *Root;
  CHAR8 Error[96];
  CHAR16 *Base;
  CHAR16 *Cursor;
  CHAR16 *Separator;
  CHAR16 FilePath[48];
  EFI_GUID EmptyGuid;
  UNIXNU_ENTRY *Entries;

  ZeroMem (Config, sizeof (*Config));
  Error[0] = '\0';
  Text = NULL;
  Handles = NULL;
  Size = 0;
  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Loaded);
  if (EFI_ERROR (Status)) return Status;
  ConfigPath = PcdGetPtr (PcdUniXNUConfigPath);
  ExplicitConfig = (Loaded->LoadOptions != NULL) &&
    (Loaded->LoadOptionsSize >= sizeof (OptionsPrefix)) &&
    (CompareMem (Loaded->LoadOptions, OptionsPrefix, sizeof (OptionsPrefix) - sizeof (CHAR16)) == 0);
  if (ExplicitConfig) {
    UINTN OptionChars = Loaded->LoadOptionsSize / sizeof (CHAR16);
    CONST CHAR16 *Options = Loaded->LoadOptions;
    if (((Loaded->LoadOptionsSize % sizeof (CHAR16)) != 0) ||
        (StrnLenS (Options, OptionChars) == OptionChars) ||
        (Options[ARRAY_SIZE (OptionsPrefix) - 1] != L'\\')) {
      return EFI_INVALID_PARAMETER;
    }
    ConfigPath = Options + ARRAY_SIZE (OptionsPrefix) - 1;
  }
  Status = UniXNUReadFile (Loaded->DeviceHandle, ConfigPath, &Text, &Size);
  if (!EFI_ERROR (Status)) {
    Config->Volume = Loaded->DeviceHandle;
  } else if (ExplicitConfig || (Status != EFI_NOT_FOUND)) {
    return Status;
  } else {
    Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &Count, &Handles);
    if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) return Status;
    if (Status == EFI_NOT_FOUND) Count = 0;
    for (Index = 0; Index < Count; Index++) {
      Status = UniXNUReadFile (Handles[Index], ConfigPath, &Text, &Size);
      if (!EFI_ERROR (Status)) {
        Config->Volume = Handles[Index];
        break;
      }
      if (Status != EFI_NOT_FOUND) {
        FreePool (Handles);
        return Status;
      }
    }
    if (Handles != NULL) FreePool (Handles);
  }
  Base = AllocateCopyPool (StrSize (ConfigPath), ConfigPath);
  if (Base == NULL) {
    if (Text != NULL) FreePool (Text);
    return EFI_OUT_OF_RESOURCES;
  }
  Separator = NULL;
  for (Cursor = Base; *Cursor != L'\0'; Cursor++) if (*Cursor == L'\\') Separator = Cursor;
  if (Separator == NULL) {
    FreePool (Base);
    if (Text != NULL) FreePool (Text);
    return EFI_INVALID_PARAMETER;
  }
  Separator[Separator == Base ? 1 : 0] = L'\0';
  Root = NULL;
  Status = TomlParse ((Text != NULL) ? Text : mFallback, (Text != NULL) ? Size : sizeof (mFallback) - 1, &Root, Error, sizeof (Error));
  if (Text != NULL) FreePool (Text);
  if (!EFI_ERROR (Status)) {
    Status = ParseConfig (Root, Base, Config);
    TomlFree (Root);
  }
  FreePool (Base);
  if (EFI_ERROR (Status)) {
    UniXNULog ("configuration failed: %r %a", Status, Error);
    return Status;
  }
  ZeroMem (&EmptyGuid, sizeof (EmptyGuid));
  if ((Config->Volume == NULL) && !CompareGuid ((EFI_GUID *)PcdGetPtr (PcdUniXNUFallbackFile), &EmptyGuid)) {
    Entries = AllocateZeroPool (2 * sizeof (*Entries));
    if (Entries == NULL) return EFI_OUT_OF_RESOURCES;
    Entries[1] = Config->Entries[0];
    FreePool (Config->Entries);
    Config->Entries = Entries;
    Config->EntryCount = 2;
    Entries[0].Id = AllocateCopyPool (sizeof ("firmware"), "firmware");
    Entries[0].Title = AllocateCopyPool (PcdGetSize (PcdUniXNUFallbackTitle), PcdGetPtr (PcdUniXNUFallbackTitle));
    UnicodeSPrint (FilePath, sizeof (FilePath), L"\\fv:%g", PcdGetPtr (PcdUniXNUFallbackFile));
    Entries[0].Path = AllocateCopyPool (StrSize (FilePath), FilePath);
    Entries[0].Type = UniXNUEntryEfi;
    if ((Entries[0].Id == NULL) || (Entries[0].Title == NULL) || (Entries[0].Path == NULL)) return EFI_OUT_OF_RESOURCES;
  }
  Config->SourceLabel = AllocateCopyPool (Config->Volume != NULL ? StrSize (ConfigPath) : sizeof (L"built-in configuration"),
                                         Config->Volume != NULL ? ConfigPath : L"built-in configuration");
  return (Config->SourceLabel == NULL) ? EFI_OUT_OF_RESOURCES : EFI_SUCCESS;
}
