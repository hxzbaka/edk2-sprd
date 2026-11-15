/** @file
  SprdUms512TcmPlatform.h

  UMS512 touch-controller platform definitions.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef SPRD_UMS512_TCM_PLATFORM_H
#define SPRD_UMS512_TCM_PLATFORM_H

#include <Uefi.h>

/*
 * Values are taken from t610.dts for the ums512_2h10 touch wiring:
 *   i2c@70600000 (alias i2c3), slave 0x22
 *   AP GPIO 144 (0x90) is the active-low interrupt, GPIO 145 (0x91) is reset
 *
 * Keep these macros overridable from the platform DSC.  Some 2h10 board
 * revisions reuse the controller but move the GPIOs or panel coordinates.
 */
#ifndef SPRD_UMS512_TCM_I2C_BASE
#define SPRD_UMS512_TCM_I2C_BASE             0x70600000U
#endif
#ifndef SPRD_UMS512_TCM_GPIO_BASE
#define SPRD_UMS512_TCM_GPIO_BASE            0x32070000U
#endif
#ifndef SPRD_UMS512_TCM_AP_APB_BASE
#define SPRD_UMS512_TCM_AP_APB_BASE           0x71000000U
#endif
#ifndef SPRD_UMS512_TCM_I2C3_EB_BIT
#define SPRD_UMS512_TCM_I2C3_EB_BIT           BIT12
#endif
#ifndef SPRD_UMS512_TCM_I2C_ADDRESS
#define SPRD_UMS512_TCM_I2C_ADDRESS          0x22U
#endif
#ifndef SPRD_UMS512_TCM_I2C_BUS_FREQUENCY
#define SPRD_UMS512_TCM_I2C_BUS_FREQUENCY    400000U
#endif
#ifndef SPRD_UMS512_TCM_I2C_SOURCE_CLOCK
#define SPRD_UMS512_TCM_I2C_SOURCE_CLOCK     26000000U
#endif
#ifndef SPRD_UMS512_TCM_IRQ_GPIO
#define SPRD_UMS512_TCM_IRQ_GPIO             144U
#endif
#ifndef SPRD_UMS512_TCM_RESET_GPIO
#define SPRD_UMS512_TCM_RESET_GPIO           145U
#endif
#ifndef SPRD_UMS512_TCM_IRQ_ACTIVE_LOW
#define SPRD_UMS512_TCM_IRQ_ACTIVE_LOW       1
#endif
// Set to one only when diagnosing GPIO144.  It blocks normal touch startup.
#ifndef SPRD_UMS512_TCM_IRQ_DIAGNOSTIC
#define SPRD_UMS512_TCM_IRQ_DIAGNOSTIC       0
#endif
#ifndef SPRD_UMS512_TCM_RESET_ACTIVE_LOW
#define SPRD_UMS512_TCM_RESET_ACTIVE_LOW     1
#endif
#ifndef SPRD_UMS512_TCM_MAX_X
#define SPRD_UMS512_TCM_MAX_X                1080U
#endif
#ifndef SPRD_UMS512_TCM_MAX_Y
#define SPRD_UMS512_TCM_MAX_Y                2340U
#endif

#define SPRD_UMS512_TCM_I2C_TIMEOUT_US       1000000U
#define SPRD_UMS512_TCM_RESET_ACTIVE_US     20000U
#define SPRD_UMS512_TCM_RESET_SETTLE_US     200000U
typedef struct {
  UINTN  Base;
  UINT32 SourceClockHz;
  UINT32 BusFrequencyHz;
  UINT8  SlaveAddress;
  BOOLEAN Ready;
} SPRD_UMS512_I2C;

typedef struct {
  UINTN Base;
  BOOLEAN Ready;
} SPRD_UMS512_GPIO;

EFI_STATUS
SprdUms512I2cInit (
  OUT SPRD_UMS512_I2C *I2c
  );

EFI_STATUS
SprdUms512I2cTransfer (
  IN OUT SPRD_UMS512_I2C *I2c,
  IN BOOLEAN Read,
  OUT UINT8 *ReadBuffer OPTIONAL,
  IN CONST UINT8 *WriteBuffer OPTIONAL,
  IN UINTN Length
  );

EFI_STATUS
SprdUms512GpioInit (
  OUT SPRD_UMS512_GPIO *Gpio
  );

EFI_STATUS
SprdUms512GpioInput (
  IN OUT SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line
  );

EFI_STATUS
SprdUms512GpioOutput (
  IN OUT SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line,
  IN BOOLEAN Value
  );

EFI_STATUS
SprdUms512GpioGet (
  IN SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line,
  OUT BOOLEAN *Value
  );

EFI_STATUS
SprdUms512TouchReset (
  IN OUT SPRD_UMS512_GPIO *Gpio
  );

#endif