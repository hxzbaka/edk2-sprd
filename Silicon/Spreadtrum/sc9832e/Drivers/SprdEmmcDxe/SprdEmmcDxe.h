/** @file
  SprdEmmcDxe.h

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef _SPRD_EMMC_DXE_H_
#define _SPRD_EMMC_DXE_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Library/PcdLib.h>

#define SDHCI_BASE                      FixedPcdGet32(PcdSdMmcPciHostControllerMmioBase)
#define AP_CLK_BASE                     FixedPcdGet32(PcdApClkBaseAddress)
#define AP_AHB_GATE_BASE                FixedPcdGet32(PcdApAhbBaseAddress)

#define AP_CLK_EMMC_2X_CFG             0x90
#define AP_AHB_EMMC_EB_OFFSET          0x00
#define AP_AHB_EMMC_EB_BIT             BIT11

#define EMMC_BASE_RATE                  384000000

#define SDHCI_32BIT_BLK_CNT             0x00
#define SDHCI_BLOCK_SIZE                0x04
#define SDHCI_BLOCK_COUNT               0x06
#define SDHCI_ARGUMENT                  0x08
#define SDHCI_TRANSFER_MODE             0x0C
#define SDHCI_COMMAND                   0x0E
#define SDHCI_RESPONSE0                 0x10
#define SDHCI_RESPONSE1                 0x14
#define SDHCI_RESPONSE2                 0x18
#define SDHCI_RESPONSE3                 0x1C
#define SDHCI_BUFFER                    0x20
#define SDHCI_PRESENT_STATE             0x24
#define SDHCI_HOST_CONTROL              0x28
#define SDHCI_POWER_CONTROL             0x29
#define SDHCI_CLOCK_CONTROL             0x2C
#define SDHCI_TIMEOUT_CONTROL           0x2E
#define SDHCI_SOFTWARE_RESET            0x2F
#define SDHCI_INT_STATUS                0x30
#define SDHCI_INT_ENABLE                0x34
#define SDHCI_SIGNAL_ENABLE             0x38
#define SDHCI_HOST_CONTROL2             0x3E
#define SDHCI_CAPABILITIES              0x40
#define SDHCI_ADMA_ERROR                0x54
#define SDHCI_ADMA_ADDRESS              0x58
#define SDHCI_ADMA_ADDRESS_HI           0x5C
#define SDHCI_HOST_VERSION              0xFE

#define SDHCI_SPRD_REG_DLL_CFG          0x200
#define SDHCI_SPRD_REG_DLL_DLY          0x204
#define SDHCI_SPRD_REG_DLL_DLY_OFFSET   0x208
#define SDHCI_SPRD_REG_DLL_STS0         0x210
#define SDHCI_SPRD_REG_BUSY_POSI        0x250
#define SDHCI_SPRD_REG_DEBOUNCE         0x28C

#define SDHCI_RESET_ALL                 0x01
#define SDHCI_RESET_CMD                 0x02
#define SDHCI_RESET_DATA                0x04
#define SDHCI_HW_RESET_CARD             BIT3

#define SDHCI_CLOCK_INT_EN              0x0001
#define SDHCI_CLOCK_INT_STABLE          0x0002
#define SDHCI_CLOCK_CARD_EN             0x0004

#define SDHCI_CMD_INHIBIT               BIT0
#define SDHCI_DATA_INHIBIT              BIT1

#define SDHCI_INT_CMD_COMPLETE          BIT0
#define SDHCI_INT_XFER_COMPLETE         BIT1
#define SDHCI_INT_DMA_INT               BIT3
#define SDHCI_INT_BUF_WR_READY          BIT4
#define SDHCI_INT_BUF_RD_READY          BIT5
#define SDHCI_INT_ERROR                 BIT15
#define SDHCI_INT_CMD_TIMEOUT           BIT16
#define SDHCI_INT_DATA_TIMEOUT          BIT20
#define SDHCI_INT_ADMA_ERROR            BIT25
#define SDHCI_INT_ALL_MASK              0xFFFFFFFF

#define SDHCI_TRNS_DMA_EN              BIT0
#define SDHCI_TRNS_BLK_CNT_EN          BIT1
#define SDHCI_TRNS_AUTO_CMD12          BIT2
#define SDHCI_TRNS_READ                BIT4
#define SDHCI_TRNS_MULTI               BIT5

#define SDHCI_CTRL_DMA_MASK            (BIT3 | BIT4)
#define SDHCI_CTRL_SDMA                0x00
#define SDHCI_CTRL_ADMA2_32           BIT4
#define SDHCI_CTRL_ADMA2_64           (BIT3|BIT4)

#define SDHCI_CTRL2_V4_MODE            BIT12
#define SDHCI_CTRL2_64BIT_ADDR         BIT13

#define SDHCI_CMD_DATA_PRESENT          BIT5

#define SDHCI_SPRD_BIT_DLL_BAK          BIT0
#define SDHCI_SPRD_BIT_DLL_VAL          BIT1
#define SDHCI_SPRD_BIT_CMD_DLY_INV      BIT13
#define SDHCI_SPRD_BIT_POSRD_DLY_INV    BIT21
#define SDHCI_SPRD_BIT_OUTR_CLK_AUTO_EN BIT25
#define SDHCI_SPRD_BIT_INNR_CLK_AUTO_EN BIT24
#define SDHCI_SPRD_CLK_MAX_DIV          1023

#define EMMC_CMD_NONE                   0x0000
#define EMMC_CMD_R1                     0x001A
#define EMMC_CMD_R1B                    0x001B
#define EMMC_CMD_R2                     0x0009
#define EMMC_CMD_R3                     0x0002

#define EMMC_RCA                        0x0001
#define EMMC_OCR_SECTOR                 BIT30
#define EMMC_OCR_VOLTAGE                0x00FF8080
#define EMMC_OCR_BUSY                   BIT31
#define EMMC_SWITCH_ACCESS_WRITE        0x03
#define EMMC_EXT_CSD_BUS_WIDTH          183
#define EMMC_EXT_CSD_HS_TIMING          185
#define EMMC_EXT_CSD_SEC_COUNT          212
#define EMMC_BUS_WIDTH_8                2
#define EMMC_HS_TIMING_HS               1
#define EMMC_BLOCK_SIZE                 512

#define CACHE_LINE_SIZE                 64

#define EMMC_MAX_XFER_BLOCKS            1024

#define BOUNCE_SIZE                     (1024 * 1024)
#define BOUNCE_BUF_BLOCKS               (BOUNCE_SIZE / EMMC_BLOCK_SIZE)

#define ADMA2_ATTR_VALID                0x01
#define ADMA2_ATTR_END                  0x02
#define ADMA2_ATTR_ACT_TRAN            0x20

#define ADMA2_MAX_DESC_COUNT            128

#pragma pack(1)
typedef struct {
  UINT16    Attr;
  UINT16    Length;
  UINT32    AddressLo;
} ADMA2_DESC_32;

typedef struct {
  UINT16    Attr;
  UINT16    Length;
  UINT32    AddressLo;
  UINT32    AddressHi;
} ADMA2_DESC_64;
#pragma pack()

EFI_STATUS SprdClockInit (VOID);
EFI_STATUS SprdSdhciReset (UINT8 Mask);
EFI_STATUS SprdSdhciSetClock (UINT32 Clock);
VOID       SprdSdhciInitConfig (VOID);
EFI_STATUS SprdSdhciSendCmd (UINT16 CmdIdx, UINT32 Arg, UINT16 RespType, UINT32 *Resp);
EFI_STATUS WaitInhibitClear (UINT32 Mask, UINT32 TimeoutUs);

#endif