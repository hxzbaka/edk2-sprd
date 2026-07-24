/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 hxzbaka
 *
 * Physical button driver for Spreadtrum, providing SimpleTextIn via
 * hardware volume up/down and power keys.
 */

#include <Uefi.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/DevicePath.h>
#include <Guid/ConsoleInDevice.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
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
UINT16
AdiRead (
  UINT32 Addr
  )
{
  UINT32 Val;

  MmioWrite32 (ADI_BASE + REG_ADI_RD_CMD, Addr);
  do {
    Val = MmioRead32 (ADI_BASE + REG_ADI_RD_DATA);
  } while (Val & BIT_RD_BUSY);

  return (UINT16)(Val & 0xFFFF);
}

//
// Periodic timer callback: scan buttons, detect edges, handle auto-repeat
//
STATIC
VOID
EFIAPI
ScanTimerHandler (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  static BOOLEAN initialized = FALSE;
  static BOOLEAN lastVolUp, lastVolDown, lastPower;
  // Hold counters (number of 100ms ticks the button has been held)
  static UINTN holdVolUp = 0, holdVolDown = 0, holdPower = 0;
  // Repeat armed flag (true after first auto-repeat sent)
  static BOOLEAN repeatVolUp = FALSE, repeatVolDown = FALSE, repeatPower = FALSE;

  UINT32 ApEic   = MmioRead32 (AP_EIC_BASE);
  UINT16 PmicEic = AdiRead (PMIC_EIC_BASE);

  BOOLEAN volUp   = !(ApEic & (1 << 2));      // active low
  BOOLEAN volDown = !!(ApEic & (1 << 10));    // active high
  BOOLEAN power   = !(PmicEic & (1 << 1));   // active low

  if (!initialized) {
    MmioWrite32 (AP_EIC_BASE + AP_EIC_MASK_OFFSET, 0xFFFFFFFF);
    lastVolUp   = volUp;
    lastVolDown = volDown;
    lastPower   = power;
    initialized = TRUE;
    return;
  }

  // --- Volume Up (SCAN_UP) ---
  if (volUp && !lastVolUp) {
    EFI_INPUT_KEY Key = { SCAN_UP, 0 };
    EnqueueKey (&Key);
    DEBUG ((DEBUG_INFO, "[BTN] VOL+ pressed\n"));
    holdVolUp = 0;
    repeatVolUp = FALSE;
  } else if (volUp && lastVolUp) {
    holdVolUp++;
    if (holdVolUp == 20) {
      EFI_INPUT_KEY Key = { SCAN_UP, 0 };
      EnqueueKey (&Key);
      repeatVolUp = TRUE;
    } else if (holdVolUp > 20 && repeatVolUp) {
      EFI_INPUT_KEY Key = { SCAN_UP, 0 };
      EnqueueKey (&Key);
    }
  } else if (!volUp && lastVolUp) {
    DEBUG ((DEBUG_INFO, "[BTN] VOL+ released\n"));
    holdVolUp = 0;
    repeatVolUp = FALSE;
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
      if (holdVolDown == 20) {
        EFI_INPUT_KEY Key = { SCAN_DOWN, 0 };
        EnqueueKey (&Key);
        DEBUG ((DEBUG_INFO, "[BTN] VOL- auto-repeat start\n"));
        repeatVolDown = TRUE;
      } else if (holdVolDown > 20 && repeatVolDown) {
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
      if (holdPower == 20) {
        EFI_INPUT_KEY Key = { 0, CHAR_CARRIAGE_RETURN };
        EnqueueKey (&Key);
        DEBUG ((DEBUG_INFO, "[BTN] POWER auto-repeat start\n"));
        repeatPower = TRUE;
      } else if (holdPower > 20 && repeatPower) {
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

/**
  Entry point: enable clocks, install protocols, start scan timer.
**/
EFI_STATUS
EFIAPI
SprdButtonDxeEntry (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  DEBUG ((DEBUG_ERROR, "[SprdButton] Starting\n"));

  // Zero queue and events (defensive)
  ZeroMem (&mQueue, sizeof(mQueue));
  mWaitEvent  = NULL;
  mTimerEvent = NULL;

  // Enable ADI/EIC clocks (same as your previous working code)
//  MmioWrite32 (0x402E0000, 0xFFFFFFFF);
//  MmioWrite32 (0x402E0010, 0xFFFFFFFF);
//  MmioWrite32 (0x402E00B0, 0xFFFFFFFF);

  // 1. Create WaitForKey event
  Status = gBS->CreateEvent (
                  0,                    // no special flags, just a simple event
                  TPL_NOTIFY,
                  NULL,                 // no notify function needed
                  NULL,
                  &mWaitEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SprdButton] CreateEvent(mWaitEvent) failed: %r\n", Status));
    return Status;
  }

  // 2. Assign event to protocol instance
  mSimpleTextIn.WaitForKey = mWaitEvent;

  // 3. Install all protocols on a new handle
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

  // 4. Optional: connect the handle so the console subsystem sees it.
  //    If this triggers a crash, comment it out – the console might be
  //    automatically detected by GuidConsoleIn.
  gBS->ConnectController (mHandle, NULL, NULL, TRUE);

  // 5. Create periodic scan timer (100 ms)
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

  Status = gBS->SetTimer (
                  mTimerEvent,
                  TimerPeriodic,
                  1000000
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SprdButton] SetTimer failed: %r\n", Status));
    return Status;
  }

  DEBUG ((DEBUG_ERROR, "[SprdButton] Ready\n"));
  return EFI_SUCCESS;
}