/** @file
  Physical button driver for Spreadtrum UMS512.

  Provides SimpleTextIn through the hardware volume and power keys.

  SPDX-License-Identifier: BSD-2-Clause

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
**/

#include <Uefi.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/DevicePath.h>
#include <Guid/ConsoleInDevice.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SprdGpioProtocol.h>
#include "SprdButtonDxe.h"

#define KEY_QUEUE_SIZE 16

typedef struct {
  EFI_INPUT_KEY Keys[KEY_QUEUE_SIZE];
  UINTN         Head;
  UINTN         Tail;
} KEY_QUEUE;

STATIC KEY_QUEUE mQueue;
STATIC EFI_EVENT mWaitEvent;
STATIC EFI_EVENT mTimerEvent;
STATIC EFI_HANDLE mHandle = NULL;
STATIC SPRD_GPIO_PROTOCOL *mGpio;

#define PLAT_KEYPAD_DEVICE_GUID \
  {0xD7F58A0E, 0xBED2, 0x4B5A, {0xBB, 0x43, 0x8A, 0xB2, 0x3D, 0xD0, 0xE2, 0xB0}}

#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH       Vendor;
  EFI_DEVICE_PATH_PROTOCOL End;
} KEYPAD_DEVICE_PATH;
#pragma pack()

STATIC KEYPAD_DEVICE_PATH mDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      { sizeof(VENDOR_DEVICE_PATH), 0 }
    },
    PLAT_KEYPAD_DEVICE_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { sizeof(EFI_DEVICE_PATH_PROTOCOL), 0 }
  }
};

STATIC
EFI_STATUS
EFIAPI
ButtonReset (
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  mQueue.Head = mQueue.Tail = 0;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ButtonReadKey (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
  OUT EFI_INPUT_KEY                  *Key
  )
{
  if (mQueue.Head == mQueue.Tail) {
    return EFI_NOT_READY;
  }
  CopyMem (Key, &mQueue.Keys[mQueue.Head], sizeof(EFI_INPUT_KEY));
  mQueue.Head = (mQueue.Head + 1) % KEY_QUEUE_SIZE;
  return EFI_SUCCESS;
}

STATIC EFI_SIMPLE_TEXT_INPUT_PROTOCOL mSimpleTextIn = {
  ButtonReset,
  ButtonReadKey,
  NULL
};

STATIC
VOID
EnqueueKey (
  IN EFI_INPUT_KEY *Key
  )
{
  UINTN Next = (mQueue.Tail + 1) % KEY_QUEUE_SIZE;

  if (Next == mQueue.Head) {
    DEBUG ((DEBUG_WARN, "[BTN] Queue full, dropping key\n"));
    return;
  }

  CopyMem (&mQueue.Keys[mQueue.Tail], Key, sizeof(EFI_INPUT_KEY));
  mQueue.Tail = Next;
  gBS->SignalEvent (mWaitEvent);
}

STATIC
VOID
EFIAPI
ScanTimerHandler (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  static BOOLEAN initialized = FALSE;
  static BOOLEAN lastVolUp;
  static BOOLEAN lastVolDown;
  static BOOLEAN lastPower;

  static UINTN holdVolUp   = 0;
  static UINTN holdVolDown = 0;
  static UINTN holdPower   = 0;

  static BOOLEAN repeatVolUp   = FALSE;
  static BOOLEAN repeatVolDown = FALSE;
  static BOOLEAN repeatPower   = FALSE;

  BOOLEAN volUp;
  BOOLEAN volDown;
  BOOLEAN power;

  if (mGpio == NULL) {
    return;
  }

  // Read raw GPIO states
  volUp   = mGpio->Get (GPIO_KEY_VOLUP);
  volDown = mGpio->Get (GPIO_KEY_VOLDOWN);
  power   = mGpio->Get (GPIO_KEY_POWER);

  /*
   * Polarity adjustment per DTS:
   *  - Vol+  : PMIC EIC4, active high -> no inversion
   *  - Vol-  : AP GPIO124, active low  -> invert
   *  - Power : PMIC EIC1, active low   -> invert
   */
  volDown = !volDown;
  power   = !power;

  if (!initialized) {
    lastVolUp   = volUp;
    lastVolDown = volDown;
    lastPower   = power;
    initialized = TRUE;
    return;
  }

  // --- Volume Up (SCAN_UP) ---
  if (volUp) {
    if (!lastVolUp) {
      EFI_INPUT_KEY Key = { SCAN_UP, 0 };
      EnqueueKey (&Key);
      DEBUG ((DEBUG_INFO, "[BTN] VOL+ pressed\n"));
      holdVolUp = 0;
      repeatVolUp = FALSE;
    } else {
      holdVolUp++;
      if (holdVolUp == 10) {
        EFI_INPUT_KEY Key = { SCAN_UP, 0 };
        EnqueueKey (&Key);
        DEBUG ((DEBUG_INFO, "[BTN] VOL+ auto-repeat start\n"));
        repeatVolUp = TRUE;
      } else if (holdVolUp > 10 && repeatVolUp) {
        EFI_INPUT_KEY Key = { SCAN_UP, 0 };
        EnqueueKey (&Key);
      }
    }
  } else {
    if (lastVolUp) {
      DEBUG ((DEBUG_INFO, "[BTN] VOL+ released\n"));
      holdVolUp = 0;
      repeatVolUp = FALSE;
    }
  }

  // --- Volume Down (SCAN_DOWN) ---
  if (volDown) {
    if (!lastVolDown) {
      EFI_INPUT_KEY Key = { SCAN_DOWN, 0 };
      EnqueueKey (&Key);
      DEBUG ((DEBUG_INFO, "[BTN] VOL- pressed\n"));
      holdVolDown = 0;
      repeatVolDown = FALSE;
    } else {
      holdVolDown++;
      if (holdVolDown == 10) {
        EFI_INPUT_KEY Key = { SCAN_DOWN, 0 };
        EnqueueKey (&Key);
        DEBUG ((DEBUG_INFO, "[BTN] VOL- auto-repeat start\n"));
        repeatVolDown = TRUE;
      } else if (holdVolDown > 10 && repeatVolDown) {
        EFI_INPUT_KEY Key = { SCAN_DOWN, 0 };
        EnqueueKey (&Key);
      }
    }
  } else {
    if (lastVolDown) {
      DEBUG ((DEBUG_INFO, "[BTN] VOL- released\n"));
      holdVolDown = 0;
      repeatVolDown = FALSE;
    }
  }

  // --- Power (Enter) ---
  if (power) {
    if (!lastPower) {
      EFI_INPUT_KEY Key = { 0, CHAR_CARRIAGE_RETURN };
      EnqueueKey (&Key);
      DEBUG ((DEBUG_INFO, "[BTN] POWER pressed\n"));
      holdPower = 0;
      repeatPower = FALSE;
    } else {
      holdPower++;
      if (holdPower == 10) {
        EFI_INPUT_KEY Key = { 0, CHAR_CARRIAGE_RETURN };
        EnqueueKey (&Key);
        DEBUG ((DEBUG_INFO, "[BTN] POWER auto-repeat start\n"));
        repeatPower = TRUE;
      } else if (holdPower > 10 && repeatPower) {
        EFI_INPUT_KEY Key = { 0, CHAR_CARRIAGE_RETURN };
        EnqueueKey (&Key);
      }
    }
  } else {
    if (lastPower) {
      DEBUG ((DEBUG_INFO, "[BTN] POWER released\n"));
      holdPower = 0;
      repeatPower = FALSE;
    }
  }

  lastVolUp   = volUp;
  lastVolDown = volDown;
  lastPower   = power;
}

EFI_STATUS
EFIAPI
SprdButtonDxeEntry (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  DEBUG ((DEBUG_ERROR, "[SprdButton] Starting\n"));

  ZeroMem (&mQueue, sizeof(mQueue));
  mWaitEvent  = NULL;
  mTimerEvent = NULL;

  // Create wait event for SimpleTextIn
  Status = gBS->CreateEvent (0, TPL_NOTIFY, NULL, NULL, &mWaitEvent);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SprdButton] CreateEvent(mWaitEvent) failed: %r\n", Status));
    return Status;
  }

  // Locate GPIO protocol
  Status = gBS->LocateProtocol (&gSprdGpioProtocolGuid, NULL, (VOID **)&mGpio);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SprdButton] GPIO protocol not found: %r\n", Status));
    return Status;
  }

  // Configure GPIO directions
  mGpio->DirectionInput (GPIO_KEY_POWER);
  mGpio->DirectionInput (GPIO_KEY_VOLUP);
  mGpio->DirectionInput (GPIO_KEY_VOLDOWN);

  /*
   * Match U-Boot's gpio-keys path.  AP GPIO124 is initialized by
   * board_keypad_init(); PMIC EIC keys are opened by sprd_eic_request()
   * immediately before sampling.  Open both PMIC inputs once here as well,
   * so a reset-to-UEFI path does not depend on a previous scan having run.
   */

  // Install SimpleTextIn and DevicePath
  mSimpleTextIn.WaitForKey = mWaitEvent;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiSimpleTextInProtocolGuid,  &mSimpleTextIn,
                  &gEfiDevicePathProtocolGuid,     &mDevicePath,
                  &gEfiConsoleInDeviceGuid,        NULL,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "InstallMultipleProtocolInterfaces failed: %r\n", Status));
    return Status;
  }

  gBS->ConnectController (mHandle, NULL, NULL, TRUE);

  // Create periodic timer for scanning keys (100ms)
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ScanTimerHandler,
                  NULL,
                  &mTimerEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SprdButton] CreateEvent(mTimerEvent) failed: %r\n", Status));
    return Status;
  }

  Status = gBS->SetTimer (mTimerEvent, TimerPeriodic, 1000000); // 100ms
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SprdButton] SetTimer failed: %r\n", Status));
    return Status;
  }

  DEBUG ((DEBUG_ERROR, "[SprdButton] Ready\n"));
  return EFI_SUCCESS;
}
