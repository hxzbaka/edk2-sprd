/** @file
  SprdPmicDxe.c

  Spreadtrum UMS512 PMIC protocol DXE driver.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Protocol/I2cMaster.h>
#include <Protocol/SprdAonI2c.h>
#include <Protocol/SprdPmic.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>

#define FAN54015_ADDRESS 0x6aU
#define FAN_REG_MONITOR  0x10U
#define FAN_REG_CONTROL0 0x00U
#define FAN_REG_CONTROL1 0x01U
#define FAN_REG_IBAT     0x04U
#define FAN_OPA_MODE     BIT0
#define FAN_HZ_MODE      BIT1
#define FAN_CHARGE_DISABLE BIT2
#define FAN_INPUT_LIMIT_MASK (BIT6 | BIT7)
#define FAN_STATUS_SHIFT 4U
#define FAN_STATUS_MASK  0x03U
#define FAN_STATUS_CHARGING 1U
#define FAN_STATUS_DONE  2U
#define SC2730_BASE       0x32120000U
#define FGU_BASE          0x32120c00U
#define FGU_VOLTAGE       0x20U
#define FGU_CURRENT       0x2cU
#define FGU_USER_AREA     0xa8U
#define SC2730_CHARGER_STATUS 0x1b9cU
#define SC2730_CHARGER_STATUS_VBUS_MASK (BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | BIT11)
#define SC2730_CHARGER_PD 0x1b98U
#define SC2730_CHARGER_PD_MASK BIT0
#define SPRD_ADI_BASE     0x32100000U
#define SPRD_ADI_RD_CMD   0x28U
#define SPRD_ADI_RD_DATA  0x2cU
#define ADI_RD_BUSY       BIT31
#define ADI_READ_TIMEOUT  2000U
#define FGU_VOLT_1V_ADC   680U
#define FGU_CUR_1A_ADC    (FGU_VOLT_1V_ADC * 2U)
#define FGU_CUR_BASE_ADC  8192
#define BATTERY_RESISTANCE_MOHM 170
#define BATTERY_LOW_MV    3300U
#define ADI_SAMPLE_PERIOD 10U

typedef struct { EFI_I2C_REQUEST_PACKET Packet; EFI_I2C_OPERATION ExtraOperation; UINT8 Data[2]; } PMIC_REQUEST;
typedef struct { SPRD_PMIC_PROTOCOL Protocol; EFI_I2C_MASTER_PROTOCOL *I2c; EFI_HANDLE Handle; } PMIC_DEVICE;
STATIC UINTN mAdiSampleCounter;
STATIC BOOLEAN mTelemetryValid;
STATIC SPRD_PMIC_BATTERY_INFO mTelemetry;
STATIC BOOLEAN mChargePathEnabled;

STATIC EFI_STATUS
PmicAdiRead16 (
  IN UINT32 Address,
  OUT UINT16 *Value
  )
{
  UINT32 Data;
  UINTN Count;

  MmioWrite32 (SPRD_ADI_BASE + SPRD_ADI_RD_CMD, Address);
  for (Count = 0; Count < ADI_READ_TIMEOUT; Count++) {
    Data = MmioRead32 (SPRD_ADI_BASE + SPRD_ADI_RD_DATA);
    if ((Data & ADI_RD_BUSY) == 0U) {
      *Value = (UINT16)Data;
      return EFI_SUCCESS;
    }
    if (gBS != NULL) {
      gBS->Stall (1);
    }
  }
  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
PmicAdiWrite16 (
  IN UINT32 Address,
  IN UINT16 Value
  )
{
  MmioWrite32 (Address, Value);
  MemoryFence ();
  if (gBS != NULL) {
    gBS->Stall (1);
  }
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
PmicAdiUpdateBits (
  IN UINT32 Address,
  IN UINT16 Mask,
  IN BOOLEAN Set
  )
{
  UINT16 Value;
  EFI_STATUS Status;

  Status = PmicAdiRead16 (Address, &Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (Set) {
    Value |= Mask;
  } else {
    Value &= (UINT16)~Mask;
  }
  return PmicAdiWrite16 (Address, Value);
}

STATIC
VOID
PmicUpdateTelemetry (
  VOID
  )
{
  UINT16 Value;
  UINT32 Raw;
  INT32 CurrentRaw;
  INT64 Compensated;

  if (EFI_ERROR (PmicAdiRead16 (FGU_BASE + FGU_VOLTAGE, &Value))) {
    return;
  }
  Raw = Value;
  if (Raw == 0U || Raw == 0xffffU) {
    return;
  }
  mTelemetry.VoltageMillivolts = (Raw * 1000U + FGU_VOLT_1V_ADC / 2U) / FGU_VOLT_1V_ADC;
  if (EFI_ERROR (PmicAdiRead16 (FGU_BASE + FGU_CURRENT, &Value))) {
    return;
  }
  CurrentRaw = (INT32)Value - FGU_CUR_BASE_ADC;
  mTelemetry.BatteryCurrentMilliamps = (CurrentRaw * 1000) / (INT32)FGU_CUR_1A_ADC;
  Compensated = (INT64)mTelemetry.VoltageMillivolts -
                ((INT64)mTelemetry.BatteryCurrentMilliamps * BATTERY_RESISTANCE_MOHM) / 1000;
  mTelemetry.CompensatedVoltageMillivolts = Compensated > 0 ? (UINT32)Compensated : 0U;
  mTelemetry.BatteryLow = mTelemetry.CompensatedVoltageMillivolts < BATTERY_LOW_MV;
  if (!EFI_ERROR (PmicAdiRead16 (FGU_BASE + FGU_USER_AREA, &Value))) {
    mTelemetry.CapacityPercent = MIN ((UINT32)Value & 0xfffU, 100U);
  }
  if (!EFI_ERROR (PmicAdiRead16 (SC2730_BASE + SC2730_CHARGER_STATUS, &Value))) {
    mTelemetry.VbusPresent = (Value & SC2730_CHARGER_STATUS_VBUS_MASK) != 0U;
  }
  mTelemetryValid = TRUE;
}

STATIC EFI_STATUS PmicTransfer(PMIC_DEVICE *Device, BOOLEAN Read, UINT8 Register, UINT8 *Value) {
  PMIC_REQUEST Request;
  if (Device == NULL || Device->I2c == NULL || Value == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem(&Request, sizeof(Request));
  Request.Packet.OperationCount = Read ? 2U : 1U;
  Request.Data[0] = Register; Request.Data[1] = *Value;
  Request.Packet.Operation[0].LengthInBytes = Read ? 1U : 2U;
  Request.Packet.Operation[0].Buffer = Request.Data;
  if (Read) { Request.ExtraOperation.Flags = I2C_FLAG_READ; Request.ExtraOperation.LengthInBytes = 1U; Request.ExtraOperation.Buffer = Value; }
  return Device->I2c->StartRequest(Device->I2c, FAN54015_ADDRESS, &Request.Packet, NULL, NULL);
}

STATIC
EFI_STATUS
EFIAPI
PmicGetBatteryInfo (
  IN SPRD_PMIC_PROTOCOL *This,
  OUT SPRD_PMIC_BATTERY_INFO *Info
  )
{
  PMIC_DEVICE *Device = BASE_CR(This, PMIC_DEVICE, Protocol);
  EFI_STATUS Status;
  UINTN Index;
  if (Info == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (Info, sizeof (*Info));
  /* The ADI controller is shared with the PMIC EIC key input.  Telemetry is
     sampled sparsely and cached; the normal one-second charger callback only
     reads the external FAN54015 over I2C. */
  for (Index = FAN_REG_CONTROL0; Index <= FAN_REG_IBAT + 2U; Index++) {
    Status = PmicTransfer (Device, TRUE, (UINT8)Index, &Info->ChargerRegisters[Index]);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  Status = PmicTransfer (Device, TRUE, FAN_REG_MONITOR, &Info->ChargerRegisters[7]);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  /* Sample the shared ADI telemetry only once every few charger ticks.
     Return the last valid sample on intervening calls. */
  if (++mAdiSampleCounter >= ADI_SAMPLE_PERIOD || !mTelemetryValid) {
    mAdiSampleCounter = 0;
    PmicUpdateTelemetry ();
  }
  if (mTelemetryValid) {
    Info->VoltageMillivolts = mTelemetry.VoltageMillivolts;
    Info->BatteryCurrentMilliamps = mTelemetry.BatteryCurrentMilliamps;
    Info->CompensatedVoltageMillivolts = mTelemetry.CompensatedVoltageMillivolts;
    Info->BatteryLow = mTelemetry.BatteryLow;
    Info->CapacityPercent = mTelemetry.CapacityPercent;
  }
  Info->VbusPresent = mTelemetry.VbusPresent;
  switch ((Info->ChargerRegisters[FAN_REG_IBAT] >> 4) & 7U) {
    case 0: Info->ChargeCurrentMilliamps = 550U; break;
    case 1: Info->ChargeCurrentMilliamps = 650U; break;
    case 2: Info->ChargeCurrentMilliamps = 750U; break;
    case 3: Info->ChargeCurrentMilliamps = 850U; break;
    case 4: Info->ChargeCurrentMilliamps = 1050U; break;
    case 5: Info->ChargeCurrentMilliamps = 1150U; break;
    case 6: Info->ChargeCurrentMilliamps = 1350U; break;
    default: Info->ChargeCurrentMilliamps = 1450U; break;
  }
  Info->ChargerStatus = (Info->ChargerRegisters[FAN_REG_CONTROL0] >> FAN_STATUS_SHIFT) & FAN_STATUS_MASK;
  /* Charging state comes only from FAN54015 REG00[5:4].  VBUS is exposed
     separately for adapter connect/remove reporting and must not be used to
     fabricate a charging state when the charger reports standby/fault. */
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI PmicSetFastCharge(SPRD_PMIC_PROTOCOL *This) {
  PMIC_DEVICE *Device = BASE_CR(This, PMIC_DEVICE, Protocol);
  UINT8 Value;
  EFI_STATUS Status;
  /* The battery-detect line is unreliable on this board.  Explicitly enable
     the SC2730 charger power path once when charging starts. */
  if (!mChargePathEnabled) {
    Status = PmicAdiUpdateBits (
               SC2730_BASE + SC2730_CHARGER_PD,
               SC2730_CHARGER_PD_MASK,
               FALSE
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mChargePathEnabled = TRUE;
  }
  Status = PmicTransfer(Device, TRUE, FAN_REG_CONTROL1, &Value); if (EFI_ERROR(Status)) return Status;
  /* FAN54015 charging mode: OPA/OTG=0, HZ=0, CE=0 (charge enabled). */
  Value &= (UINT8)~(FAN_OPA_MODE | FAN_HZ_MODE | FAN_CHARGE_DISABLE | FAN_INPUT_LIMIT_MASK);
  /* Linux's highest FAN54015 input-limit code: 2 A.  The charge-current
     code below remains capped at the chip's 1.45 A setting. */
  Value |= FAN_INPUT_LIMIT_MASK;
  Status = PmicTransfer(Device, FALSE, FAN_REG_CONTROL1, &Value); if (EFI_ERROR(Status)) return Status;
  Status = PmicTransfer(Device, TRUE, FAN_REG_IBAT, &Value); if (EFI_ERROR(Status)) return Status;
  Value = (UINT8)((Value & 0x0fU) | (7U << 4));
  Status = PmicTransfer(Device, FALSE, FAN_REG_IBAT, &Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* Linux feeds FAN54015's watchdog periodically through REG00[7]. */
  Status = PmicTransfer (Device, TRUE, FAN_REG_CONTROL0, &Value);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Value |= BIT7;
  return PmicTransfer (Device, FALSE, FAN_REG_CONTROL0, &Value);
}

STATIC EFI_STATUS EFIAPI PmicStartCharging (IN SPRD_PMIC_PROTOCOL *This) {
  return PmicSetFastCharge (This);
}

STATIC EFI_STATUS EFIAPI PmicStopCharging (IN SPRD_PMIC_PROTOCOL *This) {
  PMIC_DEVICE *Device = BASE_CR (This, PMIC_DEVICE, Protocol);
  UINT8 Value;
  EFI_STATUS Status = PmicTransfer (Device, TRUE, FAN_REG_CONTROL1, &Value);
  if (EFI_ERROR (Status)) return Status;
  Value |= BIT2;
  Status = PmicTransfer (Device, FALSE, FAN_REG_CONTROL1, &Value);
  if (!EFI_ERROR (Status)) {
    (VOID)PmicAdiUpdateBits (
              SC2730_BASE + SC2730_CHARGER_PD,
              SC2730_CHARGER_PD_MASK,
              TRUE
              );
    mChargePathEnabled = FALSE;
  }
  return Status;
}

EFI_STATUS EFIAPI SprdPmicDxeEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
  SPRD_AON_I2C_PROTOCOL *Aon; PMIC_DEVICE *Device; EFI_STATUS Status;
  (VOID)ImageHandle;
  (VOID)SystemTable;
  DEBUG ((DEBUG_INFO, "SPRD-PMIC: entry\n"));
  if (gBS == NULL) {
    return EFI_NOT_READY;
  }
  Status = gBS->LocateProtocol(&gSprdAonI2cProtocolGuid, NULL, (VOID **)&Aon); if (EFI_ERROR(Status)) return Status;
  if (Aon == NULL || Aon->I2c == NULL) {
    DEBUG ((DEBUG_ERROR, "SPRD-PMIC: AON I2C protocol is incomplete\n"));
    return EFI_DEVICE_ERROR;
  }
  DEBUG ((DEBUG_INFO, "SPRD-PMIC: AON I2C=%p\n", Aon->I2c));
  Device = AllocateZeroPool(sizeof(*Device)); if (Device == NULL) return EFI_OUT_OF_RESOURCES;
  Device->I2c = Aon->I2c;
  Device->Protocol.StartCharging = PmicStartCharging;
  Device->Protocol.StopCharging = PmicStopCharging;
  Device->Protocol.SetFastCharge = PmicSetFastCharge;
  Device->Protocol.GetBatteryInfo = PmicGetBatteryInfo;
  Status = gBS->InstallProtocolInterface(&Device->Handle, &gSprdPmicProtocolGuid, EFI_NATIVE_INTERFACE, &Device->Protocol);
  if (EFI_ERROR (Status)) {
    FreePool (Device);
  }
  return Status;
}