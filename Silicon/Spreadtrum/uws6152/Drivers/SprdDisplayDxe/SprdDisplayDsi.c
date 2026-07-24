/** @file
  DSI controller initialization and generic write commands

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdDisplayDxe.h"

#define DSI_SOFT_RESET               0x0004
#define DSI_MASK_PROTOCOL_INT        0x000C
#define DSI_MASK_INTERNAL_INT        0x0014
#define DSI_MODE_CFG                 0x0018
#define DSI_VIRTUAL_CHANNEL_ID       0x001C
#define DSI_DPI_VIDEO_FORMAT         0x0020
#define DSI_TX_ESCAPE_CLOCK_CONFIG   0x005C
#define DSI_CMD_MODE_CFG             0x0068
#define DSI_PHY_CLKLANE_TIME_CONFIG  0x00A8
#define DSI_PHY_DATALANE_TIME_CONFIG 0x00AC
#define DSI_MAX_READ_TIME            0x00B0
#define DSI_RX_PKT_CHECK_CONFIG      0x00B4
#define DSI_TA_EN                    0x00B8
#define DSI_EOTP_EN                  0x00BC
#define DSI_DCS_WM_PKT_SIZE          0x00C4
#define DSI_GEN_HDR                  0x006C
#define DSI_GEN_PLD_DATA             0x0070
#define DSI_CMD_MODE_STATUS          0x0098

#define DSI_MODE_COMMAND          1U
#define DSI_COLOR_CODE_24BIT      5U
#define DSI_VIDEO_FORMAT_MASK     0x3FU
#define DSI_RX_ECC_ENABLE         BIT0
#define DSI_RX_CRC_ENABLE         BIT1
#define DSI_BTA_ENABLE            BIT0
#define DSI_CMD_TEAR_ACK_ENABLE   BIT0
#define DSI_CMD_LP_TRANSMIT_MASK  0x010F7F00U
#define DSI_CMD_WRITE_DATA_EMPTY  BIT3
#define DSI_CMD_FIFO_EMPTY        BIT5

#define MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM 0x03U
#define MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM 0x13U
#define MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM 0x23U
#define MIPI_DSI_GENERIC_LONG_WRITE          0x29U

#define DSI_DATA_HS_TO_LP_NS 120U
#define DSI_DATA_LP_TO_HS_NS 500U
#define DSI_CLK_HS_TO_LP_NS  4U
#define DSI_CLK_LP_TO_HS_NS  15U
#define DSI_MAX_READ_TIME_NS 6000U

STATIC
UINT32
DivideRoundUp(IN UINT32 Value, IN UINT32 Divisor)
{
  return (Value + Divisor - 1) / Divisor;
}

STATIC
UINT32
NanosecondsToByteCycles(IN UINT32 Nanoseconds, IN UINT32 ByteClockKhz)
{
  return DivideRoundUp(Nanoseconds * ByteClockKhz, 1000000U);
}

STATIC
VOID
DsiPowerEnable(IN CONST SPRD_DISPLAY_CONFIG *Config, IN BOOLEAN Enable)
{
  MmioWrite32(Config->DsiBase + DSI_SOFT_RESET, Enable ? 1U : 0U);
}

STATIC
VOID
DsiConfigureEdpi(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  DsiPowerEnable(Config, FALSE);
  MmioAndThenOr32(
      Config->DsiBase + DSI_DPI_VIDEO_FORMAT,
      ~DSI_VIDEO_FORMAT_MASK,
      DSI_COLOR_CODE_24BIT);
  MmioAnd32(Config->DsiBase + DSI_CMD_MODE_CFG, ~DSI_CMD_TEAR_ACK_ENABLE);
  MmioWrite32(Config->DsiBase + DSI_DCS_WM_PKT_SIZE, Config->Width);
  MmioWrite32(Config->DsiBase + DSI_MASK_PROTOCOL_INT, 0xFFFFFFFFU);
  MmioWrite32(Config->DsiBase + DSI_MASK_INTERNAL_INT, 0xFFFFFFFFU);
  DsiPowerEnable(Config, TRUE);
}

STATIC
EFI_STATUS
DsiWaitCommandStatus(
    IN CONST SPRD_DISPLAY_CONFIG *Config,
    IN UINT32                     Mask)
{
  UINT32 Timeout;

  for (Timeout = 0; Timeout < 5000; Timeout++) {
    if ((MmioRead32(Config->DsiBase + DSI_CMD_MODE_STATUS) & Mask) != 0) {
      return EFI_SUCCESS;
    }

    gBS->Stall(1);
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
SprdDsiInitialize(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 ByteClock;
  UINT32 ClockTiming;
  UINT32 DataTiming;
  UINT32 EscapeDivider;

  ByteClock = Config->PhyBitClock / 8;
  if ((ByteClock == 0) || (Config->EscapeClock == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  DsiPowerEnable(Config, FALSE);
  MmioWrite32(Config->DsiBase + DSI_MASK_PROTOCOL_INT, 0xFFFFFFFFU);
  MmioWrite32(Config->DsiBase + DSI_MASK_INTERNAL_INT, 0xFFFFFFFFU);
  MmioWrite32(Config->DsiBase + DSI_MODE_CFG, DSI_MODE_COMMAND);
  MmioWrite32(Config->DsiBase + DSI_EOTP_EN, 0);
  MmioWrite32(
      Config->DsiBase + DSI_RX_PKT_CHECK_CONFIG,
      DSI_RX_ECC_ENABLE | DSI_RX_CRC_ENABLE);
  MmioWrite32(Config->DsiBase + DSI_TA_EN, DSI_BTA_ENABLE);
  MmioWrite32(Config->DsiBase + DSI_VIRTUAL_CHANNEL_ID, 0);

  EscapeDivider = DivideRoundUp(ByteClock, Config->EscapeClock);
  MmioWrite32(
      Config->DsiBase + DSI_TX_ESCAPE_CLOCK_CONFIG,
      EscapeDivider);

  MmioWrite32(
      Config->DsiBase + DSI_MAX_READ_TIME,
      NanosecondsToByteCycles(DSI_MAX_READ_TIME_NS, ByteClock));

  DataTiming =
      NanosecondsToByteCycles(DSI_DATA_LP_TO_HS_NS, ByteClock) |
      (NanosecondsToByteCycles(DSI_DATA_HS_TO_LP_NS, ByteClock) << 16);
  ClockTiming =
      NanosecondsToByteCycles(DSI_CLK_LP_TO_HS_NS, ByteClock) |
      (NanosecondsToByteCycles(DSI_CLK_HS_TO_LP_NS, ByteClock) << 16);
  MmioWrite32(Config->DsiBase + DSI_PHY_DATALANE_TIME_CONFIG, DataTiming);
  MmioWrite32(Config->DsiBase + DSI_PHY_CLKLANE_TIME_CONFIG, ClockTiming);
  DsiPowerEnable(Config, TRUE);

  DsiConfigureEdpi(Config);
  return EFI_SUCCESS;
}

VOID
SprdDsiEnableLpCommands(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioWrite32(Config->DsiBase + DSI_MODE_CFG, DSI_MODE_COMMAND);
  MmioOr32(Config->DsiBase + DSI_CMD_MODE_CFG, DSI_CMD_LP_TRANSMIT_MASK);
}

VOID
SprdDsiPrepareForTransfer(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  MmioWrite32(Config->DsiBase + DSI_MODE_CFG, DSI_MODE_COMMAND);
  DsiPowerEnable(Config, FALSE);
  gBS->Stall(100);
  DsiPowerEnable(Config, TRUE);
}

EFI_STATUS
SprdDsiGenericWrite(
    IN CONST SPRD_DISPLAY_CONFIG *Config,
    IN CONST UINT8               *Payload,
    IN UINTN                      PayloadLength)
{
  EFI_STATUS Status;
  UINT32     DataType;
  UINT32     Header;
  UINT32     Index;
  UINT32     PayloadWord;
  UINT32     WordByte;

  if ((Payload == NULL) || (PayloadLength > MAX_UINT16)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = DsiWaitCommandStatus(Config, DSI_CMD_WRITE_DATA_EMPTY);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (PayloadLength > 2) {
    for (Index = 0; Index < PayloadLength; Index += 4) {
      PayloadWord = 0;
      for (WordByte = 0;
           (WordByte < 4) && ((Index + WordByte) < PayloadLength);
           WordByte++) {
        PayloadWord |=
            (UINT32)Payload[Index + WordByte] << (WordByte * 8);
      }

      MmioWrite32(Config->DsiBase + DSI_GEN_PLD_DATA, PayloadWord);
    }

    DataType = MIPI_DSI_GENERIC_LONG_WRITE;
    Header = DataType | ((UINT32)PayloadLength << 8);
  } else {
    if (PayloadLength == 0) {
      DataType = MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM;
      Header   = DataType;
    } else if (PayloadLength == 1) {
      DataType = MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM;
      Header   = DataType | ((UINT32)Payload[0] << 8);
    } else {
      DataType = MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM;
      Header = DataType | ((UINT32)Payload[0] << 8) |
          ((UINT32)Payload[1] << 16);
    }
  }

  Status = DsiWaitCommandStatus(Config, DSI_CMD_FIFO_EMPTY);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  MmioWrite32(Config->DsiBase + DSI_GEN_HDR, Header);
  return EFI_SUCCESS;
}
