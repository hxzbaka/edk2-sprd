/** @file
  SprdChargerDxe.c

  Spreadtrum UMS512 charger DXE driver.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Guid/EventGroup.h>
#include <Protocol/SprdPmic.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
typedef struct {
  SPRD_PMIC_PROTOCOL *Pmic;
  EFI_EVENT           Timer;
  EFI_EVENT           ExitBoot;
  BOOLEAN             HaveVbusState;
  BOOLEAN             LastVbusPresent;
} CHARGER_CONTEXT;
STATIC VOID EFIAPI ChargerTimer(IN EFI_EVENT Event, IN VOID *Context) {
  CHARGER_CONTEXT *Charger = Context;
  SPRD_PMIC_BATTERY_INFO Info;
  EFI_STATUS Status;
  Status = Charger->Pmic->SetFastCharge(Charger->Pmic);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_WARN, "SPRD-CHARGER: watchdog refresh failed: %r\n", Status));
  }

  Status = Charger->Pmic->GetBatteryInfo(Charger->Pmic, &Info);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_WARN, "SPRD-CHARGER: PMIC read failed: %r\n", Status));
    return;
  }

  if (!Charger->HaveVbusState ||
      Charger->LastVbusPresent != Info.VbusPresent) {
    DEBUG((DEBUG_ERROR, Info.VbusPresent
           ? "SPRD-CHARGER: charger connected\n"
           : "SPRD-CHARGER: charger removed\n"));
    Charger->LastVbusPresent = Info.VbusPresent;
    Charger->HaveVbusState = TRUE;
  }

  /* Do not continuously dump battery/charger telemetry while unplugged. */
  if (!Info.VbusPresent) {
    return;
  }

  DEBUG((DEBUG_INFO, "SPRD-CHARGER: VBAT=%umV IBAT=%dmA CAP=%u%% ICHG=%umA VBUS=%u STATUS=%u CHARGING=%u R00=0x%02x R01=0x%02x R04=0x%02x\n",
         Info.VoltageMillivolts, Info.BatteryCurrentMilliamps,
         Info.CapacityPercent, Info.ChargeCurrentMilliamps,
         Info.VbusPresent, Info.ChargerStatus,
         (Info.ChargerStatus == 1U), Info.ChargerRegisters[0],
         Info.ChargerRegisters[1], Info.ChargerRegisters[4]));
}
STATIC VOID EFIAPI ChargerExitBoot(IN EFI_EVENT Event, IN VOID *Context) {
  CHARGER_CONTEXT *Charger = Context;
  EFI_STATUS Status;
  SPRD_PMIC_BATTERY_INFO Info;
  (VOID)Event;
  if (Charger == NULL || Charger->Pmic == NULL) return;
  if (Charger->Timer != NULL) (VOID)gBS->SetTimer(Charger->Timer, TimerCancel, 0);
  Status = Charger->Pmic->GetBatteryInfo(Charger->Pmic, &Info);
  if (!EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "SPRD-CHARGER: EBS VBAT=%umV IBAT=%dmA ICHG=%umA VBUS=%u STATUS=%u\n",
           Info.VoltageMillivolts, Info.BatteryCurrentMilliamps,
           Info.ChargeCurrentMilliamps, Info.VbusPresent, Info.ChargerStatus));
  }
  Status = Charger->Pmic->StopCharging(Charger->Pmic);
  DEBUG ((EFI_ERROR(Status) ? DEBUG_WARN : DEBUG_INFO,
          "SPRD-CHARGER: ExitBootServices stop charging: %r\n", Status));
}
EFI_STATUS EFIAPI SprdChargerDxeEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  CHARGER_CONTEXT *Charger; EFI_STATUS Status;
  (VOID)ImageHandle;
  (VOID)SystemTable;
  DEBUG ((DEBUG_INFO, "SPRD-CHARGER: entry\n"));
  Charger = AllocateZeroPool(sizeof(*Charger)); if (Charger == NULL) return EFI_OUT_OF_RESOURCES;
  Status = gBS->LocateProtocol(&gSprdPmicProtocolGuid, NULL, (VOID **)&Charger->Pmic); if (EFI_ERROR(Status)) { FreePool(Charger); return Status; }
  Status = Charger->Pmic->StartCharging(Charger->Pmic);
  DEBUG((DEBUG_INFO, "SPRD-CHARGER: charging enable: %r\n", Status));
  if (EFI_ERROR (Status)) { FreePool (Charger); return Status; }
  Status = gBS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, ChargerTimer, Charger, &Charger->Timer);
  if (!EFI_ERROR(Status)) Status = gBS->SetTimer(Charger->Timer, TimerPeriodic, 10000000ULL);
  if (!EFI_ERROR(Status)) Status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, ChargerExitBoot, Charger, &gEfiEventExitBootServicesGuid, &Charger->ExitBoot);
  if (EFI_ERROR(Status)) { Charger->Pmic->StopCharging (Charger->Pmic); if (Charger->Timer != NULL) gBS->CloseEvent (Charger->Timer); FreePool(Charger); }
  return Status;
}