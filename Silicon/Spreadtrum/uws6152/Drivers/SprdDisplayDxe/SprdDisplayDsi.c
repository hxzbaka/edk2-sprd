/** @file
  SprdDisplayDsi.c

  DSI controller initialization and generic write commands

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
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
#define DSI_VIDEO_PKT_CONFIG         0x0024
#define DSI_VIDEO_LINE_HBLK_TIME     0x0028
#define DSI_VIDEO_LINE_TIME          0x002C
#define DSI_VIDEO_VBLK_LINES         0x0030
#define DSI_VIDEO_VACTIVE_LINES      0x0034
#define DSI_VID_MODE_CFG             0x0038
#define DSI_TIMEOUT_CNT_CLK_CONFIG   0x0040
#define DSI_HTX_TO_CONFIG            0x0044
#define DSI_LRX_H_TO_CONFIG          0x0048
#define DSI_TX_ESCAPE_CLOCK_CONFIG   0x005C
#define DSI_CMD_MODE_CFG             0x0068
#define DSI_PHY_CLKLANE_TIME_CONFIG  0x00A8
#define DSI_PHY_DATALANE_TIME_CONFIG 0x00AC
#define DSI_MAX_READ_TIME            0x00B0
#define DSI_RX_PKT_CHECK_CONFIG      0x00B4
#define DSI_TA_EN                    0x00B8
#define DSI_EOTP_EN                  0x00BC
#define DSI_VIDEO_NULLPKT_SIZE       0x00C0
#define DSI_DCS_WM_PKT_SIZE          0x00C4
#define DSI_VIDEO_SIG_DELAY_CONFIG   0x00D0
#define DSI_GEN_HDR                  0x006C
#define DSI_GEN_PLD_DATA             0x0070
#define DSI_CMD_MODE_STATUS          0x0098

#define DSI_MODE_VIDEO            0U
#define DSI_MODE_COMMAND          1U
#define DSI_COLOR_CODE_24BIT      5U
#define DSI_VIDEO_FORMAT_MASK     0x3FU
#define DSI_VIDEO_LP_PORCH_ENABLE   0x00003F00U
#define DSI_RGB888_BYTES_PER_PIXEL_X100 300U
#define DSI_PACKET_HEADER_BYTES       6U
#define DSI_NULL_PACKET_MAX           1023U
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
DsiConfigureDpi(IN CONST SPRD_DISPLAY_CONFIG *Config)
{
  UINT32 ByteClock;
  UINT32 BytesLeft;
  UINT32 BytesPerChunk;
  UINT32 ChunkOverhead;
  UINT32 Chunks;
  UINT32 HorizontalTotal;
  UINT32 NullPacketSize;
  UINT32 PixelClockKhz;
  UINT32 RatioX1000;
  UINT32 TimeoutCount;
  UINT32 TimeoutDivider;
  UINT32 TotalBytes;
  UINT32 VideoSize;

  ByteClock = Config->PhyBitClock / 8;
  PixelClockKhz = Config->PixelClock / 1000;
  if ((ByteClock == 0) || (PixelClockKhz == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  HorizontalTotal = Config->Width + Config->HSync +
      Config->HBackPorch + Config->HFrontPorch;
  RatioX1000 = ByteClock * 1000 / PixelClockKhz;
  VideoSize = Config->Width;
  Chunks = 0;
  NullPacketSize = 0;

  if (Config->VideoModeType < 2) {
    BytesPerChunk = Config->Width * DSI_RGB888_BYTES_PER_PIXEL_X100 / 100 +
        DSI_PACKET_HEADER_BYTES;
    TotalBytes = (Config->Width + Config->HFrontPorch) * RatioX1000 *
        Config->Lanes / 1000 * 100 / 120;
    if (TotalBytes < BytesPerChunk) {
      return EFI_UNSUPPORTED;
    }

    ChunkOverhead = TotalBytes - BytesPerChunk;
    if (ChunkOverhead > 1) {
      BytesLeft = 0;
      for (VideoSize = 1; VideoSize < Config->Width; VideoSize++) {
        if (((Config->Width * 1000 / VideoSize) % 1000) != 0) {
          continue;
        }

        Chunks = Config->Width / VideoSize;
        BytesPerChunk =
            DSI_RGB888_BYTES_PER_PIXEL_X100 * VideoSize / 100 +
            DSI_PACKET_HEADER_BYTES;
        if (TotalBytes >= (BytesPerChunk * Chunks)) {
          BytesLeft = TotalBytes - BytesPerChunk * Chunks;
          break;
        }
      }

      if (BytesLeft > (DSI_PACKET_HEADER_BYTES * Chunks)) {
        NullPacketSize =
            (BytesLeft - DSI_PACKET_HEADER_BYTES * Chunks) / Chunks;
        if (NullPacketSize > DSI_NULL_PACKET_MAX) {
          NullPacketSize = DSI_NULL_PACKET_MAX;
        }
      }
    } else {
      Chunks = 1;
    }
  }

  DsiPowerEnable(Config, FALSE);
  MmioAndThenOr32(
      Config->DsiBase + DSI_DPI_VIDEO_FORMAT,
      ~DSI_VIDEO_FORMAT_MASK,
      DSI_COLOR_CODE_24BIT);
  MmioWrite32(
      Config->DsiBase + DSI_VID_MODE_CFG,
      Config->VideoModeType | DSI_VIDEO_LP_PORCH_ENABLE);
  MmioWrite32(
      Config->DsiBase + DSI_VIDEO_SIG_DELAY_CONFIG,
      95 * HorizontalTotal * RatioX1000 / 100000);
  MmioWrite32(
      Config->DsiBase + DSI_VIDEO_LINE_TIME,
      HorizontalTotal * RatioX1000 / 1000);
  MmioWrite32(
      Config->DsiBase + DSI_VIDEO_LINE_HBLK_TIME,
      (Config->HBackPorch * RatioX1000 / 1000) |
      ((Config->HSync * RatioX1000 / 1000) << 16));
  MmioWrite32(
      Config->DsiBase + DSI_VIDEO_VBLK_LINES,
      Config->VFrontPorch | (Config->VBackPorch << 10) |
      (Config->VSync << 20));
  MmioWrite32(Config->DsiBase + DSI_VIDEO_VACTIVE_LINES, Config->Height);

  TimeoutCount = HorizontalTotal * Config->Height +
      (2 * DSI_RGB888_BYTES_PER_PIXEL_X100) / 100;
  for (TimeoutDivider = 0x80;
       (TimeoutDivider < TimeoutCount) && (TimeoutDivider > 2);
       TimeoutDivider--) {
    if ((TimeoutCount % TimeoutDivider) == 0) {
      MmioWrite32(
          Config->DsiBase + DSI_TIMEOUT_CNT_CLK_CONFIG,
          TimeoutDivider);
      MmioWrite32(
          Config->DsiBase + DSI_LRX_H_TO_CONFIG,
          TimeoutCount / TimeoutDivider);
      MmioWrite32(
          Config->DsiBase + DSI_HTX_TO_CONFIG,
          TimeoutCount / TimeoutDivider);
      break;
    }
  }

  MmioWrite32(
      Config->DsiBase + DSI_VIDEO_PKT_CONFIG,
      VideoSize | (Chunks << 16));
  MmioWrite32(
      Config->DsiBase + DSI_VIDEO_NULLPKT_SIZE,
      NullPacketSize);
  MmioWrite32(Config->DsiBase + DSI_MASK_PROTOCOL_INT, 0xFFFFFFFFU);
  MmioWrite32(Config->DsiBase + DSI_MASK_INTERNAL_INT, 0xFFFFFFFFU);
  DsiPowerEnable(Config, TRUE);
  return EFI_SUCCESS;
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

  if (!Config->VideoMode) {
    DsiConfigureEdpi(Config);
    return EFI_SUCCESS;
  }

  return DsiConfigureDpi(Config);
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
  MmioWrite32(
      Config->DsiBase + DSI_MODE_CFG,
      Config->VideoMode ? DSI_MODE_VIDEO : DSI_MODE_COMMAND);
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