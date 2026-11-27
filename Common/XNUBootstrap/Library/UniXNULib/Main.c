// Copyright (c) 2026, UniXNU contributors. All rights reserved.
// SPDX-License-Identifier: BSD-2-Clause-Patent
#include "Internal.h"
#include <Protocol/SerialIo.h>

STATIC EFI_SERIAL_IO_PROTOCOL *mSerial;

VOID EFIAPI
UniXNULog (CONST CHAR8 *Format, ...)
{
  VA_LIST Args;
  CHAR8 Line[256];
  UINTN Size;

  AsciiStrCpyS (Line, sizeof (Line), "UNIXNU: ");
  VA_START (Args, Format);
  AsciiVSPrint (Line + 8, sizeof (Line) - 10, Format, Args);
  VA_END (Args);
  AsciiStrCatS (Line, sizeof (Line), "\r\n");
  DEBUG ((PcdGet32 (PcdUniXNUDebugLevel), "%a", Line));
  if (mSerial != NULL) {
    Size = AsciiStrLen (Line);
    mSerial->Write (mSerial, &Size, Line);
  }
}

EFI_STATUS EFIAPI
UniXNURun (EFI_HANDLE ImageHandle, UNIXNU_HOST Host)
{
  EFI_STATUS Status;
  UNIXNU_CONFIG Config;
  UNIXNU_UI Ui;
  UNIXNU_ENTRY *Entry;
  UNIXNU_ENTRY Copy;
  EFI_HANDLE *Handles;
  UINTN Count;
  UINTN Index;
  UINTN Choice;
  BOOLEAN Countdown;

  gBS->SetWatchdogTimer (0, 0, 0, NULL);
  if (EFI_ERROR (gBS->LocateProtocol (&gEfiSerialIoProtocolGuid, NULL, (VOID **)&mSerial))) {
    mSerial = NULL;
  }
  if ((Host == UniXNUHostFirmware) &&
      !EFI_ERROR (gBS->LocateHandleBuffer (AllHandles, NULL, NULL, &Count, &Handles))) {
    for (Index = 0; Index < Count; Index++) {
      gBS->ConnectController (Handles[Index], NULL, NULL, TRUE);
    }
    FreePool (Handles);
  }
  Status = UniXNUConfigLoad (ImageHandle, &Config);
  if (EFI_ERROR (Status)) {
    UniXNUConfigFree (&Config);
    return Status;
  }
  UniXNUUiInit (&Config, &Ui);
  Countdown = TRUE;
  for (;;) {
    Index = UniXNUMenuRun (&Ui, &Config, Countdown);
    if (Index == MAX_UINTN) {
      UniXNUConfigFree (&Config);
      return EFI_ABORTED;
    }
    Entry = &Config.Entries[Index];
    Copy = *Entry;
    if (Entry->Confirm != NULL) {
      if (!UniXNUMenuConfirm (&Ui, Entry, &Choice)) {
        Countdown = FALSE;
        continue;
      }
      if (Entry->ConfirmCount != 0) {
        UINTN Length = (Entry->Args != NULL ? StrLen (Entry->Args) : 0) + StrLen (Entry->ConfirmArgs[Choice]) + 2;
        Copy.Args = AllocateZeroPool (Length * sizeof (CHAR16));
        if (Copy.Args == NULL) {
          UnicodeSPrint (Ui.Notice, sizeof (Ui.Notice), L"%r", EFI_OUT_OF_RESOURCES);
          Countdown = FALSE;
          continue;
        }
        UnicodeSPrint (Copy.Args, Length * sizeof (CHAR16), L"%s %s", Entry->Args != NULL ? Entry->Args : L"", Entry->ConfirmArgs[Choice]);
      }
    }
    Status = UniXNUBootEntry (ImageHandle, &Copy);
    if (Copy.Args != Entry->Args) {
      FreePool (Copy.Args);
    }
    if ((Entry->Type == UniXNUEntryAction) && (Entry->Action == UniXNUActionExit)) {
      UniXNUConfigFree (&Config);
      return EFI_SUCCESS;
    }
    UnicodeSPrint (Ui.Notice, sizeof (Ui.Notice), L"'%s' returned: %r", Entry->Title, Status);
    UniXNULog ("entry %a returned %r", Entry->Id, Status);
    gBS->SetWatchdogTimer (0, 0, 0, NULL);
    Countdown = FALSE;
  }
}
