/** @file UMS512 battery charger service protocol. */
#ifndef SPRD_PMIC_PROTOCOL_H_
#define SPRD_PMIC_PROTOCOL_H_
#include <Uefi.h>
#define SPRD_PMIC_PROTOCOL_GUID \
  { 0x6aa1859a, 0x0783, 0x4952, { 0xa9, 0xf9, 0x5e, 0x24, 0x2c, 0x99, 0xd1, 0x0f } }
extern EFI_GUID gSprdPmicProtocolGuid;
typedef struct {
  UINT32 VoltageMillivolts;
  INT32 BatteryCurrentMilliamps;
  UINT32 CompensatedVoltageMillivolts;
  BOOLEAN BatteryLow;
  UINT32 CapacityPercent;
  UINT32 ChargeCurrentMilliamps;
  UINT8 ChargerStatus;
  BOOLEAN VbusPresent;
  UINT8 ChargerRegisters[8]; /* FAN54015 R0..R6 and monitor R10. */
} SPRD_PMIC_BATTERY_INFO;
typedef struct _SPRD_PMIC_PROTOCOL SPRD_PMIC_PROTOCOL;
typedef EFI_STATUS (EFIAPI *SPRD_PMIC_START_CHARGING)(IN SPRD_PMIC_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *SPRD_PMIC_STOP_CHARGING)(IN SPRD_PMIC_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *SPRD_PMIC_SET_FAST_CHARGE)(IN SPRD_PMIC_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *SPRD_PMIC_GET_BATTERY_INFO)(IN SPRD_PMIC_PROTOCOL *This, OUT SPRD_PMIC_BATTERY_INFO *Info);
struct _SPRD_PMIC_PROTOCOL {
  SPRD_PMIC_START_CHARGING StartCharging;
  SPRD_PMIC_STOP_CHARGING StopCharging;
  SPRD_PMIC_SET_FAST_CHARGE SetFastCharge;
  SPRD_PMIC_GET_BATTERY_INFO GetBatteryInfo;
};
#endif
