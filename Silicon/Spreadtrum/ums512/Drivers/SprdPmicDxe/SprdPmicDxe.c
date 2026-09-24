/** @file
  Spreadtrum UMS512 PMIC protocol DXE driver.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
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
#define FGU_BASE         0x32120c00U
#define FGU_VOLTAGE      0x20U
#define FGU_CURRENT      0x2cU
#define FGU_USER_AREA    0xa8U
#define SC2730_BASE       0x32120000U
#define SC2730_MODULE_EN0 0x1808U
#define SC2730_CLK_EN0    0x1810U
#define SC2730_CHARGER_PD 0x1b98U
#define SC2730_CHARGER_PD_MASK BIT0
#define SC2730_CHARGER_STATUS 0x1b9cU
#define SC2730_CHARGER_STATUS_VBUS_MASK (BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | BIT11)
#define SC2730_FGU_EN     BIT7
#define SC2730_FGU_RTC_EN BIT6
#define FGU_VOLT_1V_ADC   2872U
#define FGU_CUR_1A_ADC    (FGU_VOLT_1V_ADC * 2U)
#define FGU_CUR_BASE_ADC  8192
#define FGU_FALLBACK_1V_ADC 2872U
#define SPRD_ADI_BASE     0x32100000U
#define SPRD_ADI_CHNL_PRIL 0x08U
#define SPRD_ADI_CHNL_PRIH 0x0cU
#define SPRD_ADI_RD_CMD   0x28U
#define SPRD_ADI_RD_DATA  0x2cU
#define ADI_RD_BUSY       BIT31
#define ADI_READ_TIMEOUT  2000U
#define ADI_READ_RETRIES  4U
#define EFUSE_BASE         (SC2730_BASE + 0x300U)
#define EFUSE_MODULE_EN    BIT6
#define EFUSE_DATA_RD      0x04U
#define EFUSE_BLOCK_INDEX  0x0cU
#define EFUSE_MODE_CTRL    0x10U
#define EFUSE_STATUS       0x14U
#define EFUSE_RD_START     BIT1
#define EFUSE_CLR_RDDONE   BIT2
#define EFUSE_STANDBY      BIT2
#define EFUSE_RD_DONE      BIT4
#define EFUSE_FGU_BLOCK    3U
#define BATTERY_RESISTANCE_MOHM 170
#define BATTERY_LOW_MV    3300U

typedef struct { EFI_I2C_REQUEST_PACKET Packet; EFI_I2C_OPERATION ExtraOperation; UINT8 Data[2]; } PMIC_REQUEST;
typedef struct { SPRD_PMIC_PROTOCOL Protocol; EFI_I2C_MASTER_PROTOCOL *I2c; EFI_HANDLE Handle; } PMIC_DEVICE;

STATIC BOOLEAN mAdiReady;
STATIC BOOLEAN mFguVoltageCalibrationRead;
STATIC UINT32 mFguVoltage1vAdc = FGU_FALLBACK_1V_ADC;

STATIC EFI_STATUS
SprdAdiRead16 (
  IN UINT32 Address,
  OUT UINT16 *Value
  );

STATIC
VOID
PmicAdiUpdateBits (
  IN UINT16 Register,
  IN UINT16 Mask,
  IN BOOLEAN Set
  )
{
  UINT16 Value;

  if (EFI_ERROR (SprdAdiRead16 ((UINT32)(SC2730_BASE + Register), &Value))) {
    return;
  }
  if (Set) {
    Value |= Mask;
  } else {
    Value &= (UINT16)~Mask;
  }
  /* UMS512 uses ADI_15BIT_MODE; write through the SC2730 slave aperture. */
  MmioWrite32 (SC2730_BASE + Register, Value);
}

STATIC
EFI_STATUS
SprdAdiRead16 (
  IN UINT32 Address,
  OUT UINT16 *Value
  )
{
  UINT32 Data;
  UINTN Count;

  if (Value == NULL || Address < 0x32120000U || Address > 0x3212ffffU) {
    return EFI_INVALID_PARAMETER;
  }
  if (!mAdiReady) {
    /* UMS512 uses ADI R3: priorities are programmed before RD_CMD is used. */
    MmioWrite32 (SPRD_ADI_BASE + SPRD_ADI_CHNL_PRIL, 0U);
    MmioWrite32 (SPRD_ADI_BASE + SPRD_ADI_CHNL_PRIH, 0U);
    mAdiReady = TRUE;
  }
  MmioWrite32 (SPRD_ADI_BASE + SPRD_ADI_RD_CMD, Address);
  for (Count = 0; Count < ADI_READ_TIMEOUT; Count++) {
    Data = MmioRead32 (SPRD_ADI_BASE + SPRD_ADI_RD_DATA);
    if ((Data & ADI_RD_BUSY) == 0U) {
      *Value = (UINT16)(Data & 0xffffU);
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
SprdAdiReadStable16 (
  IN UINT32 Address,
  OUT UINT16 *Value
  )
{
  EFI_STATUS Status;
  UINT16 Previous;
  UINT16 Current;
  UINTN Attempt;

  Status = SprdAdiRead16 (Address, &Previous);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Attempt = 0; Attempt < 8U; Attempt++) {
    Status = SprdAdiRead16 (Address, &Current);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (Current == Previous) {
      *Value = Current;
      return EFI_SUCCESS;
    }
    Previous = Current;
  }
  *Value = Previous;
  return EFI_SUCCESS;
}

STATIC
VOID
PmicReadFguVoltageCalibration (
  VOID
  )
{
  EFI_STATUS Status;
  UINT16 Value;
  UINTN Count;
  UINT32 Cal4200MvAdc;

  if (mFguVoltageCalibrationRead) {
    return;
  }
  mFguVoltageCalibrationRead = TRUE;

  /* Linux reads this SC2730 fgu_calib cell from eFuse block 3. */
  MmioOr32 (SC2730_BASE + SC2730_MODULE_EN0, EFUSE_MODULE_EN);
  for (Count = 0; Count < ADI_READ_TIMEOUT; Count++) {
    Status = SprdAdiRead16 (EFUSE_BASE + EFUSE_STATUS, &Value);
    if (!EFI_ERROR (Status) && (Value & EFUSE_STANDBY) != 0U) {
      break;
    }
    gBS->Stall (1);
  }
  if (Count == ADI_READ_TIMEOUT) {
    goto Exit;
  }
  MmioWrite32 (EFUSE_BASE + EFUSE_BLOCK_INDEX, EFUSE_FGU_BLOCK);
  MmioWrite32 (EFUSE_BASE + EFUSE_MODE_CTRL, EFUSE_RD_START);
  for (Count = 0; Count < ADI_READ_TIMEOUT; Count++) {
    Status = SprdAdiRead16 (EFUSE_BASE + EFUSE_STATUS, &Value);
    if (!EFI_ERROR (Status) && (Value & EFUSE_RD_DONE) != 0U) {
      break;
    }
    gBS->Stall (1);
  }
  if (Count != ADI_READ_TIMEOUT &&
      !EFI_ERROR (SprdAdiRead16 (EFUSE_BASE + EFUSE_DATA_RD, &Value))) {
    Cal4200MvAdc = (Value & 0x1ffU) + 2611U;
    mFguVoltage1vAdc = (Cal4200MvAdc * 10U + 21U) / 42U;
    DEBUG ((DEBUG_INFO, "SPRD-PMIC: FGU voltage eFuse=0x%04x, 1V=%u ADC\n", Value, mFguVoltage1vAdc));
  }

Exit:
  MmioWrite32 (EFUSE_BASE + EFUSE_MODE_CTRL, EFUSE_CLR_RDDONE);
  MmioAnd32 (SC2730_BASE + SC2730_MODULE_EN0, ~EFUSE_MODULE_EN);
}

STATIC
VOID
PmicEnableFgu (
  VOID
  )
{
  MmioOr32 (SC2730_BASE + SC2730_MODULE_EN0, SC2730_FGU_EN);
  MmioOr32 (SC2730_BASE + SC2730_CLK_EN0, SC2730_FGU_RTC_EN);
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
  UINT16 AdiValue;
  UINT32 Raw;
  INT32 CurrentRaw;
  INT64 Compensated;

  if (Info == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (Info, sizeof (*Info));
  PmicReadFguVoltageCalibration ();
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
  Status = SprdAdiReadStable16 (FGU_BASE + FGU_VOLTAGE, &AdiValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "SPRD-PMIC: FGU voltage ADI read failed: %r\n", Status));
    Info->BatteryLow = TRUE;
    return EFI_SUCCESS;
  }
  Raw = AdiValue;
  if (Raw == 0U || Raw == 0xffffU) {
    DEBUG ((DEBUG_WARN, "SPRD-PMIC: FGU voltage invalid raw=0x%04x\n", Raw));
    Info->BatteryLow = TRUE;
    return EFI_SUCCESS;
  }
  Info->VoltageMillivolts = (Raw * 1000U + (mFguVoltage1vAdc / 2U)) / mFguVoltage1vAdc;
  /* U-Boot leaves the live current ADC at 0x2c valid; 0xf0 is zero on this board. */
  Status = SprdAdiReadStable16 (FGU_BASE + FGU_CURRENT, &AdiValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CurrentRaw = (INT32)AdiValue - FGU_CUR_BASE_ADC;
  if (CurrentRaw >= 0) {
    Info->BatteryCurrentMilliamps = (CurrentRaw * 1000 + (INT32)(FGU_CUR_1A_ADC / 2U)) / (INT32)FGU_CUR_1A_ADC;
  } else {
    Info->BatteryCurrentMilliamps = (CurrentRaw * 1000 - (INT32)(FGU_CUR_1A_ADC / 2U)) / (INT32)FGU_CUR_1A_ADC;
  }
  Compensated = (INT64)Info->VoltageMillivolts -
                ((INT64)Info->BatteryCurrentMilliamps * BATTERY_RESISTANCE_MOHM) / 1000;
  Info->CompensatedVoltageMillivolts = (Compensated > 0) ? (UINT32)Compensated : 0U;
  Info->BatteryLow = Info->CompensatedVoltageMillivolts < BATTERY_LOW_MV;
  Status = SprdAdiReadStable16 (FGU_BASE + FGU_USER_AREA, &AdiValue);
  Info->CapacityPercent = EFI_ERROR (Status) ? 0U : MIN ((UINT32)AdiValue & 0xfffU, 100U);
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
  /* FAN54015 has no reliable VBUS bit in REG00.  The SC2730 charger
     detector is the source used by Linux/U-Boot for adapter presence. */
  if (!EFI_ERROR (SprdAdiRead16 (SC2730_BASE + SC2730_CHARGER_STATUS, &AdiValue))) {
    Info->VbusPresent = (AdiValue & SC2730_CHARGER_STATUS_VBUS_MASK) != 0;
    DEBUG ((DEBUG_VERBOSE, "SPRD-PMIC: CHGR_STATUS=0x%04x VBUS=%u\n",
      AdiValue, Info->VbusPresent));
  } else {
    Info->VbusPresent = FALSE;
  }
  if (Info->VbusPresent && Info->ChargerStatus == 0) {
    Info->ChargerStatus = FAN_STATUS_CHARGING;
  }
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI PmicSetFastCharge(SPRD_PMIC_PROTOCOL *This) {
  PMIC_DEVICE *Device = BASE_CR(This, PMIC_DEVICE, Protocol);
  UINT16 ChargerPd;
  UINT8 Value;
  EFI_STATUS Status;
  PmicAdiUpdateBits (SC2730_CHARGER_PD, SC2730_CHARGER_PD_MASK, FALSE);
  if (!EFI_ERROR (SprdAdiRead16 (SC2730_BASE + SC2730_CHARGER_PD, &ChargerPd))) {
    DEBUG ((DEBUG_INFO, "SPRD-PMIC: SC2730 CHGR_CTRL=0x%04x after enable\n", ChargerPd));
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
    PmicAdiUpdateBits (SC2730_CHARGER_PD, SC2730_CHARGER_PD_MASK, TRUE);
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
  PmicEnableFgu ();
  Status = gBS->InstallProtocolInterface(&Device->Handle, &gSprdPmicProtocolGuid, EFI_NATIVE_INTERFACE, &Device->Protocol);
  if (EFI_ERROR (Status)) {
    FreePool (Device);
  }
  return Status;
}
