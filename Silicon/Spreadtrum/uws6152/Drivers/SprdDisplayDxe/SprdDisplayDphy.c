/** @file
  SprdDisplayDphy.c

  Megacores DPHY initialization and PLL configuration

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdDisplayDxe.h"

#define DSI_PHY_CLK_LANE_LP_CTRL 0x0074
#define DSI_PHY_INTERFACE_CTRL   0x0078
#define DSI_PHY_STATUS           0x009C
#define DSI_PHY_MIN_STOP_TIME    0x00A0
#define DSI_PHY_LANE_NUM_CONFIG  0x00A4
#define DSI_PHY_TST_CTRL0        0x00F0
#define DSI_PHY_TST_CTRL1        0x00F4

#define DPHY_SHUTDOWN       BIT0
#define DPHY_RESET_N        BIT1
#define DPHY_CLOCK_ENABLE   BIT2
#define DPHY_HS_CLOCK       BIT0
#define DPHY_PLL_LOCKED     BIT1
#define DPHY_TEST_CLEAR     BIT0
#define DPHY_TEST_CLOCK     BIT1
#define DPHY_TEST_ENABLE    BIT16
#define DPHY_TEST_DATA_MASK 0xFFU

#define DPHY_REF_CLOCK_MHZ 26U
#define DPHY_VCO_LOW_MHZ   1250U
#define DPHY_VCO_MID_MHZ   1800U
#define DPHY_VCO_HIGH_MHZ  2500U

STATIC CONST UINT8 DataLaneTimingBases[] = {
  0x41, 0x51, 0x61, 0x71, 0xA0, 0xB0, 0xC0, 0xD0
};

STATIC
UINT32
DivideRoundUp(IN UINT32 Value, IN UINT32 Divisor)
{
  return (Value + Divisor - 1) / Divisor;
}

STATIC
UINT32
Average(IN UINT32 First, IN UINT32 Second)
{
  return MIN(First, Second) + ((MAX(First, Second) - MIN(First, Second)) / 2);
}

STATIC
VOID
DphyTestClock(IN CONST SPRD_DISPLAY_CONFIG *Config, IN BOOLEAN High)
{
  if (High) {
    MmioOr32(Config->DsiBase + DSI_PHY_TST_CTRL0, DPHY_TEST_CLOCK);
  } else {
    MmioAnd32(Config->DsiBase + DSI_PHY_TST_CTRL0, ~DPHY_TEST_CLOCK);
  }
}

STATIC
VOID
DphyTestClear(IN CONST SPRD_DISPLAY_CONFIG *Config, IN BOOLEAN Assert)
{
  if (Assert) {
    MmioOr32(Config->DsiBase + DSI_PHY_TST_CTRL0, DPHY_TEST_CLEAR);
  } else {
    MmioAnd32(Config->DsiBase + DSI_PHY_TST_CTRL0, ~DPHY_TEST_CLEAR);
  }
}

STATIC
VOID
DphyTestWrite(
    IN CONST SPRD_DISPLAY_CONFIG *Config,
    IN UINT8                      Address,
    IN UINT8                      Data)
{
  MmioOr32(Config->DsiBase + DSI_PHY_TST_CTRL1, DPHY_TEST_ENABLE);
  MmioAndThenOr32(
      Config->DsiBase + DSI_PHY_TST_CTRL1,
      ~DPHY_TEST_DATA_MASK,
      Address);
  DphyTestClock(Config, TRUE);
  DphyTestClock(Config, FALSE);

  MmioAnd32(Config->DsiBase + DSI_PHY_TST_CTRL1, ~DPHY_TEST_ENABLE);
  MmioAndThenOr32(
      Config->DsiBase + DSI_PHY_TST_CTRL1,
      ~DPHY_TEST_DATA_MASK,
      Data);
  DphyTestClock(Config, TRUE);
  DphyTestClock(Config, FALSE);
}

STATIC
EFI_STATUS
DphyConfigurePll(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Fvco;
  UINT32 Kint;
  UINT32 Nint;
  UINT32 OutputSelect;
  UINT32 PostDivider;
  UINT32 PotentialFvco;
  UINT32 VcoBand;

  PotentialFvco = Config->PhyBitClock / 1000;
  Fvco          = 0;
  OutputSelect  = 0;
  for (PostDivider = 0; PostDivider < 4; PostDivider++) {
    if ((PotentialFvco >= DPHY_VCO_LOW_MHZ) &&
        (PotentialFvco <= DPHY_VCO_HIGH_MHZ)) {
      Fvco         = PotentialFvco;
      OutputSelect = 1U << PostDivider;
      break;
    }

    PotentialFvco <<= 1;
  }

  if (Fvco == 0) {
    return EFI_UNSUPPORTED;
  }

  VcoBand = (Fvco > DPHY_VCO_MID_MHZ) ? 1U : 0U;
  Nint    = Fvco / DPHY_REF_CLOCK_MHZ;
  Kint = (UINT32)DivU64x32(
      LShiftU64((UINT64)(Fvco % DPHY_REF_CLOCK_MHZ), 20),
      DPHY_REF_CLOCK_MHZ);

  DphyTestWrite(Config, 0x03, BIT0 | BIT1);
  DphyTestWrite(Config, 0x04, BIT3 | (2U << 5) | BIT7);
  DphyTestWrite(Config, 0x07, 0);
  DphyTestWrite(Config, 0x08, VcoBand | BIT1 | (3U << 2));
  DphyTestWrite(Config, 0x09, (UINT8)(Kint >> 12));
  DphyTestWrite(Config, 0x0A, (UINT8)(Kint >> 4));
  DphyTestWrite(Config, 0x0B, (UINT8)(OutputSelect | ((Kint & 0xFU) << 4)));
  DphyTestWrite(Config, 0x0C, 0);
  DphyTestWrite(Config, 0x0D, 0);
  DphyTestWrite(Config, 0x0E, BIT4);
  DphyTestWrite(Config, 0x0F, 1);
  DphyTestWrite(Config, 0x06, (UINT8)Nint);

  return EFI_SUCCESS;
}

STATIC
VOID
DphyWriteLaneTiming(
    IN CONST SPRD_DISPLAY_CONFIG *Config,
    IN UINT8                      ClockAddress0,
    IN UINT8                      ClockAddress1,
    IN UINT8                      DataOffset,
    IN UINT8                      ClockValue,
    IN UINT8                      DataValue,
    IN BOOLEAN                    WriteData)
{
  UINT32 Index;

  DphyTestWrite(Config, ClockAddress0, ClockValue);
  DphyTestWrite(Config, ClockAddress1, ClockValue);
  if (!WriteData) {
    return;
  }

  for (Index = 0; Index < ARRAY_SIZE(DataLaneTimingBases); Index++) {
    DphyTestWrite(
        Config,
        (UINT8)(DataLaneTimingBases[Index] + DataOffset),
        DataValue);
  }
}

STATIC
EFI_STATUS
DphyConfigureTiming(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 ByteClockTime;
  UINT32 ClockPrepare;
  UINT32 Constant;
  UINT32 DataPrepare;
  UINT32 HalfByteClockTime;
  UINT32 RangeLow;
  UINT32 Ui;
  UINT8  ClockValue;
  UINT8  DataValue;

  if ((Config->PhyBitClock / 1000) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  Ui                = 100000U / (Config->PhyBitClock / 1000);
  ByteClockTime     = Ui << 3;
  HalfByteClockTime = Ui << 2;
  Constant          = Ui << 1;

  ClockValue = (UINT8)(DivideRoundUp(50U * 100U * 4U, ByteClockTime) - 2U);
  DataValue  = ClockValue;
  DphyWriteLaneTiming(Config, 0x31, 0x90, 0, ClockValue, DataValue, TRUE);

  ClockPrepare = Average(38U * 100U, 95U * 100U);
  ClockValue   = (UINT8)(DivideRoundUp(ClockPrepare, HalfByteClockTime) - 1U);
  DataPrepare  = Average(40U * 100U + 4U * Ui, 85U * 100U + 6U * Ui);
  DataValue    = (UINT8)(DivideRoundUp(DataPrepare, HalfByteClockTime) - 1U);
  DphyWriteLaneTiming(Config, 0x32, 0x91, 1, ClockValue, DataValue, TRUE);

  ClockValue = (UINT8)(DivideRoundUp(
      300U * 100U * 2U + ClockPrepare - 525U * ByteClockTime / 100U,
      ByteClockTime) - 2U);
  DataValue = (UINT8)(DivideRoundUp(
      (145U * 100U + 10U * Ui) * 2U + DataPrepare -
          525U * ByteClockTime / 100U,
      ByteClockTime) - 2U);
  DphyWriteLaneTiming(Config, 0x33, 0x92, 2, ClockValue, DataValue, TRUE);

  ClockValue = (UINT8)DivideRoundUp(
      60U * 100U * 2U - Constant,
      HalfByteClockTime);
  RangeLow = MAX(8U * Ui, 60U * 100U + 4U * Ui);
  DataValue = (UINT8)(DivideRoundUp(
      RangeLow * 3U / 2U - Constant,
      HalfByteClockTime) - 2U);
  DphyWriteLaneTiming(Config, 0x34, 0x93, 3, ClockValue, DataValue, TRUE);

  ClockValue = (UINT8)(DivideRoundUp(100U * 100U * 2U, ByteClockTime) - 2U);
  DataValue  = ClockValue;
  DphyWriteLaneTiming(Config, 0x36, 0x95, 5, ClockValue, DataValue, TRUE);

  RangeLow   = 60U * 100U + 52U * Ui;
  ClockValue = (UINT8)(DivideRoundUp(RangeLow * 2U, ByteClockTime) - 2U);
  DphyWriteLaneTiming(Config, 0x35, 0x94, 0, ClockValue, 0, FALSE);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DphyWaitPllLocked(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 Retry;

  for (Retry = 0; Retry < 50000; Retry++) {
    if ((MmioRead32(Config->DsiBase + DSI_PHY_STATUS) & DPHY_PLL_LOCKED) != 0) {
      return EFI_SUCCESS;
    }

    gBS->Stall(3);
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
SprdDphyInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  EFI_STATUS Status;

  if ((Config->Lanes == 0) || (Config->Lanes > 4)) {
    return EFI_INVALID_PARAMETER;
  }

  MmioAnd32(
      Config->DsiBase + DSI_PHY_INTERFACE_CTRL,
      ~(DPHY_RESET_N | DPHY_SHUTDOWN | DPHY_CLOCK_ENABLE));

  DphyTestClear(Config, FALSE);
  DphyTestClear(Config, TRUE);
  DphyTestClear(Config, FALSE);

  Status = DphyConfigurePll(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = DphyConfigureTiming(Config);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  MmioOr32(
      Config->DsiBase + DSI_PHY_INTERFACE_CTRL,
      DPHY_SHUTDOWN | DPHY_RESET_N);
  MmioWrite32(Config->DsiBase + DSI_PHY_MIN_STOP_TIME, 0x1C);
  MmioOr32(Config->DsiBase + DSI_PHY_INTERFACE_CTRL, DPHY_CLOCK_ENABLE);
  MmioWrite32(Config->DsiBase + DSI_PHY_LANE_NUM_CONFIG, Config->Lanes - 1);

  return DphyWaitPllLocked(Config);
}

EFI_STATUS
SprdDphyEnableHsClock(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioAnd32(Config->DsiBase + DSI_PHY_CLK_LANE_LP_CTRL, ~BIT1);
  MmioOr32(Config->DsiBase + DSI_PHY_CLK_LANE_LP_CTRL, DPHY_HS_CLOCK);
  return DphyWaitPllLocked(Config);
}