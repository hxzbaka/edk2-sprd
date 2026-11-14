/** @file
  SprdEmmcDxe.c

  SPRD eMMC Block I/O Driver

  Copyright (c) 2026, kanadenadi. All rights reserved.<BR>
  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Library/CacheMaintenanceLib.h>
#include <Protocol/DevicePath.h>
#include "SprdEmmcDxe.h"

STATIC EFI_BLOCK_IO_PROTOCOL  mBlockIo;
STATIC EFI_BLOCK_IO_MEDIA     mMedia;
STATIC EFI_HANDLE             mHandle = NULL;
STATIC UINT32                 mRca;
STATIC ADMA2_DESC_64          *mAdmaDescTable = NULL;
STATIC UINT8                  *mBounceBuffer  = NULL;


#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH          Vendor;
  EFI_DEVICE_PATH_PROTOCOL    End;
} SPRD_EMMC_DEVICE_PATH;
#pragma pack()


STATIC SPRD_EMMC_DEVICE_PATH mDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)sizeof (VENDOR_DEVICE_PATH),
        (UINT8)(sizeof (VENDOR_DEVICE_PATH) >> 8)
      }
    },
    { 0x3EB4C147, 0x83B9, 0x0C39, { 0x04, 0x79, 0xEE, 0xB8, 0x0F, 0xC7, 0x52, 0x28 } }
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { sizeof (EFI_DEVICE_PATH_PROTOCOL), 0 }
  }
};



STATIC EFI_STATUS
AdmaAllocResources (VOID)
{
  EFI_STATUS           Status;
  EFI_PHYSICAL_ADDRESS PhysAddr;

  if (mAdmaDescTable == NULL) {
    PhysAddr = 0xFFFFFFFF;
    Status = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, 1, &PhysAddr);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mAdmaDescTable = (ADMA2_DESC_64 *)(UINTN)PhysAddr;
    ZeroMem (mAdmaDescTable, EFI_PAGE_SIZE);
    WriteBackDataCacheRange (mAdmaDescTable, EFI_PAGE_SIZE);
  }

  if (mBounceBuffer == NULL) {
    PhysAddr = 0xFFFFFFFF;
    UINTN Pages = EFI_SIZE_TO_PAGES (BOUNCE_BUF_BLOCKS * EMMC_BLOCK_SIZE);
    Status = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, Pages, &PhysAddr);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mBounceBuffer = (UINT8 *)(UINTN)PhysAddr;
  }

  return EFI_SUCCESS;
}

STATIC EFI_STATUS
AdmaBuildDescTable (
  IN VOID   *Buf,
  IN UINTN  XferSize
  )
{
  UINT32  Idx;
  UINTN   Remaining;
  UINTN   ChunkLen;
  UINT64  Addr64;

  Addr64    = (UINT64)(UINTN)Buf;
  Remaining = XferSize;
  Idx       = 0;

  while (Remaining > 0 && Idx < ADMA2_MAX_DESC_COUNT) {
    ChunkLen = (Remaining >= 65536) ? 65536 : Remaining;

    mAdmaDescTable[Idx].Attr      = ADMA2_ATTR_ACT_TRAN | ADMA2_ATTR_VALID;
    mAdmaDescTable[Idx].Length    = (UINT16)(ChunkLen & 0xFFFF);
    mAdmaDescTable[Idx].AddressLo = (UINT32)(Addr64 & 0xFFFFFFFF);
    mAdmaDescTable[Idx].AddressHi = (UINT32)(Addr64 >> 32);
    mAdmaDescTable[Idx].Reserve   = 0;

    Addr64    += ChunkLen;
    Remaining -= ChunkLen;
    Idx++;
  }

  if (Remaining > 0) {
    return EFI_BUFFER_TOO_SMALL;
  }
  if (Idx == 0) {
    return EFI_INVALID_PARAMETER;
  }

  mAdmaDescTable[Idx - 1].Attr |= ADMA2_ATTR_END;

  WriteBackDataCacheRange (mAdmaDescTable, sizeof (ADMA2_DESC_64) * Idx);

  return EFI_SUCCESS;
}

STATIC VOID
AdmaSetHostCtrl (VOID)
{
  UINT8  HostCtrl;
  UINT16 Ctrl2;

  Ctrl2  = MmioRead16 (SDHCI_BASE + SDHCI_HOST_CONTROL2);
  Ctrl2 |= SDHCI_CTRL2_64BIT_ADDR;
  MmioWrite16 (SDHCI_BASE + SDHCI_HOST_CONTROL2, Ctrl2);

  HostCtrl  = MmioRead8 (SDHCI_BASE + SDHCI_HOST_CONTROL);
  HostCtrl &= ~(UINT8)SDHCI_CTRL_DMA_MASK;
  HostCtrl |= (UINT8)SDHCI_CTRL_ADMA2_64;

  MmioWrite8 (SDHCI_BASE + SDHCI_HOST_CONTROL, HostCtrl);

  MmioWrite32 (SDHCI_BASE + SDHCI_ADMA_ADDRESS, (UINT32)(UINTN)mAdmaDescTable);
  MmioWrite32 (SDHCI_BASE + SDHCI_ADMA_ADDRESS_HI, (UINT32)((UINT64)(UINTN)mAdmaDescTable >> 32));
}

STATIC VOID
HostRestoreAfterReset (VOID)
{
  MmioWrite32 (SDHCI_BASE + SDHCI_INT_ENABLE, 0x03FF003B);
  MmioWrite32 (SDHCI_BASE + SDHCI_SIGNAL_ENABLE, 0);
  MmioWrite8  (SDHCI_BASE + SDHCI_TIMEOUT_CONTROL, 0x0E);
  AdmaSetHostCtrl ();
}

STATIC EFI_STATUS
EmmcWaitReady (VOID)
{
  UINT32     Resp[4];
  EFI_STATUS Status;
  UINT32     Timeout;

  for (Timeout = 0; Timeout < 1000; Timeout++) {
    Status = SprdSdhciSendCmd (13, mRca << 16, EMMC_CMD_R1, Resp);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if ((Resp[0] & 0x1E00) == 0x0800) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay (1000);
  }

  return EFI_TIMEOUT;
}

STATIC EFI_STATUS
EmmcSendDataCmdRaw (
  UINT16   CmdIdx,
  UINT32   Arg,
  VOID     *Buf,
  UINT32   BlkCount,
  BOOLEAN  IsRead
  )
{
  EFI_STATUS Status;
  UINT16     CmdReg;
  UINT16     TransMode;
  UINT32     IntSts;
  UINT32     j;
  UINTN      XferSize;

  XferSize = (UINTN)BlkCount * EMMC_BLOCK_SIZE;

  Status = WaitInhibitClear (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT, 100000);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, SDHCI_INT_ALL_MASK);
  MicroSecondDelay (10);

  MmioWrite8  (SDHCI_BASE + SDHCI_TIMEOUT_CONTROL, 0x0E);
  AdmaSetHostCtrl ();

  Status = AdmaBuildDescTable (Buf, XferSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (IsRead) {
    InvalidateDataCacheRange (Buf, XferSize);
  } else {
    WriteBackDataCacheRange (Buf, XferSize);
  }

  MmioWrite32 (SDHCI_BASE + SDHCI_ADMA_ADDRESS, (UINT32)(UINTN)mAdmaDescTable);
  MmioWrite32 (SDHCI_BASE + SDHCI_ADMA_ADDRESS_HI, (UINT32)((UINT64)(UINTN)mAdmaDescTable >> 32));

  MmioWrite16 (SDHCI_BASE + SDHCI_BLOCK_SIZE, (UINT16)(EMMC_BLOCK_SIZE | (7 << 12)));
  {
    UINT16 HostVer = MmioRead16 (SDHCI_BASE + SDHCI_HOST_VERSION);
    UINT8  SpecVer = (UINT8)(HostVer & 0x0F);
    if (SpecVer >= 3) {
      MmioOr16 (SDHCI_BASE + SDHCI_HOST_CONTROL2, SDHCI_CTRL2_V4_MODE);
      MmioWrite32 (SDHCI_BASE + SDHCI_32BIT_BLK_CNT, BlkCount);
    } else {
      MmioWrite16 (SDHCI_BASE + SDHCI_BLOCK_COUNT, (UINT16)(BlkCount & 0xFFFF));
    }
  }

  TransMode = SDHCI_TRNS_DMA_EN | SDHCI_TRNS_BLK_CNT_EN;
  if (BlkCount > 1) {
    TransMode |= SDHCI_TRNS_MULTI | SDHCI_TRNS_AUTO_CMD12;
  }
  if (IsRead) {
    TransMode |= SDHCI_TRNS_READ;
  }

  MmioWrite32 (SDHCI_BASE + SDHCI_ARGUMENT, Arg);
  CmdReg = (CmdIdx << 8) | EMMC_CMD_R1 | SDHCI_CMD_DATA_PRESENT;
  MmioWrite32 (SDHCI_BASE + SDHCI_TRANSFER_MODE, (UINT32)TransMode | ((UINT32)CmdReg << 16));

  for (j = 0; j < 10000000; j++) {
    IntSts = MmioRead32 (SDHCI_BASE + SDHCI_INT_STATUS);

    if (IntSts & SDHCI_INT_ERROR) {
      DEBUG ((DEBUG_ERROR, "SprdEmmc: CMD%d err=0x%08x adma=0x%08x ps=0x%08x\n",
        CmdIdx, IntSts,
        MmioRead32 (SDHCI_BASE + SDHCI_ADMA_ERROR),
        MmioRead32 (SDHCI_BASE + SDHCI_PRESENT_STATE)));
      MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, SDHCI_INT_ALL_MASK);
      MmioWrite16 (SDHCI_BASE + SDHCI_TRANSFER_MODE, 0);
      SprdSdhciReset (SDHCI_RESET_CMD | SDHCI_RESET_DATA);
      HostRestoreAfterReset ();
      return EFI_DEVICE_ERROR;
    }

    if (IntSts & SDHCI_INT_XFER_COMPLETE) {
      MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, SDHCI_INT_ALL_MASK);
      MmioWrite16 (SDHCI_BASE + SDHCI_TRANSFER_MODE, 0);
      if (IsRead) {
        InvalidateDataCacheRange (Buf, XferSize);
      }
      return EFI_SUCCESS;
    }

    if (IntSts & SDHCI_INT_DMA_INT) {
      MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, SDHCI_INT_DMA_INT);
    }

    MicroSecondDelay (1);
  }

  MmioWrite16 (SDHCI_BASE + SDHCI_TRANSFER_MODE, 0);
  SprdSdhciReset (SDHCI_RESET_CMD | SDHCI_RESET_DATA);
  HostRestoreAfterReset ();
  return EFI_TIMEOUT;
}

STATIC EFI_STATUS
EmmcSendDataCmd (
  UINT16   CmdIdx,
  UINT32   Arg,
  VOID     *Buf,
  UINT32   BlkCount,
  BOOLEAN  IsRead
  )
{
  EFI_STATUS Status;
  UINT32     Resp[4];

  Status = EmmcSendDataCmdRaw (CmdIdx, Arg, Buf, BlkCount, IsRead);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  SprdSdhciSendCmd (12, 0, EMMC_CMD_R1B, Resp);
  MicroSecondDelay (2000);

  SprdSdhciSendCmd (13, mRca << 16, EMMC_CMD_R1, Resp);
  if (!EFI_ERROR (SprdSdhciSendCmd (13, mRca << 16, EMMC_CMD_R1, Resp))) {
    UINT32 State = (Resp[0] >> 9) & 0xF;
    if (State != 4) {
      SprdSdhciSendCmd (7, 0, EMMC_CMD_R1B, Resp);
      MicroSecondDelay (1000);
      SprdSdhciSendCmd (7, mRca << 16, EMMC_CMD_R1B, Resp);
      MicroSecondDelay (5000);
    }
  }

  return EmmcSendDataCmdRaw (CmdIdx, Arg, Buf, BlkCount, IsRead);
}

STATIC EFI_STATUS
EmmcSwitch (UINT8 Index, UINT8 Value)
{
  UINT32     Arg;
  UINT32     Resp[4];
  EFI_STATUS Status;

  Arg = ((UINT32)EMMC_SWITCH_ACCESS_WRITE << 24) |
        ((UINT32)Index << 16) |
        ((UINT32)Value << 8);

  Status = SprdSdhciSendCmd (6, Arg, EMMC_CMD_R1B, Resp);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MicroSecondDelay (10000);
  return EmmcWaitReady ();
}

STATIC EFI_STATUS
EmmcInitCardFull (VOID)
{
  EFI_STATUS Status;
  UINT32     Resp[4];
  UINT32     Ocr;
  UINT32     Timeout;
  UINT8      *ExtCsd;
  UINT32     SecCount;
  UINT8      HostCtrl;

  SprdSdhciReset (SDHCI_RESET_ALL);

  MmioWrite8  (SDHCI_BASE + SDHCI_POWER_CONTROL, 0x0B);
  MmioWrite8  (SDHCI_BASE + SDHCI_TIMEOUT_CONTROL, 0x0E);
  MmioWrite32 (SDHCI_BASE + SDHCI_INT_ENABLE, 0x03FF003B);
  MmioWrite32 (SDHCI_BASE + SDHCI_SIGNAL_ENABLE, 0);

  AdmaSetHostCtrl ();

  Status = SprdSdhciSetClock (400000);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  MicroSecondDelay (10000);

  SprdSdhciSendCmd (0, 0, EMMC_CMD_NONE, NULL);
  MicroSecondDelay (2000);

  for (Timeout = 0; Timeout < 1000; Timeout++) {
    Status = SprdSdhciSendCmd (1, EMMC_OCR_SECTOR | EMMC_OCR_VOLTAGE, EMMC_CMD_R3, Resp);
    if (EFI_ERROR (Status)) {
      MicroSecondDelay (1000);
      continue;
    }
    Ocr = Resp[0];
    if (Ocr & EMMC_OCR_BUSY) {
      break;
    }
    MicroSecondDelay (1000);
  }

  if (Timeout >= 1000) {
    return EFI_TIMEOUT;
  }

  Status = SprdSdhciSendCmd (2, 0, EMMC_CMD_R2, Resp);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mRca = EMMC_RCA;
  Status = SprdSdhciSendCmd (3, mRca << 16, EMMC_CMD_R1, Resp);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SprdSdhciSendCmd (9, mRca << 16, EMMC_CMD_R2, Resp);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SprdSdhciSendCmd (7, mRca << 16, EMMC_CMD_R1B, Resp);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  MicroSecondDelay (5000);

  ExtCsd = (UINT8 *)(UINTN)mBounceBuffer;
  ZeroMem (ExtCsd, EMMC_BLOCK_SIZE);

  Status = EmmcSendDataCmd (8, 0, ExtCsd, 1, TRUE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SecCount = ExtCsd[EMMC_EXT_CSD_SEC_COUNT] |
             ((UINT32)ExtCsd[EMMC_EXT_CSD_SEC_COUNT + 1] << 8) |
             ((UINT32)ExtCsd[EMMC_EXT_CSD_SEC_COUNT + 2] << 16) |
             ((UINT32)ExtCsd[EMMC_EXT_CSD_SEC_COUNT + 3] << 24);

  DEBUG ((DEBUG_INFO, "SprdEmmc: BW=%d HS=%d SEC=%u (%uMB)\n",
    ExtCsd[EMMC_EXT_CSD_BUS_WIDTH], ExtCsd[EMMC_EXT_CSD_HS_TIMING],
    SecCount, SecCount / 2048));

  if (SecCount == 0) {
    return EFI_DEVICE_ERROR;
  }

  Status = EmmcSwitch (EMMC_EXT_CSD_BUS_WIDTH, EMMC_BUS_WIDTH_8);
  if (!EFI_ERROR (Status)) {
    HostCtrl  = MmioRead8 (SDHCI_BASE + SDHCI_HOST_CONTROL);
    HostCtrl &= ~((UINT8)SDHCI_CTRL_DMA_MASK | (UINT8)BIT1);
    HostCtrl |= (UINT8)BIT5;
    if (MmioRead16 (SDHCI_BASE + SDHCI_HOST_CONTROL2) & SDHCI_CTRL2_64BIT_ADDR) {
      HostCtrl |= (UINT8)SDHCI_CTRL_ADMA2_64;
    } else {
      HostCtrl |= (UINT8)SDHCI_CTRL_ADMA2_32;
    }
    MmioWrite8 (SDHCI_BASE + SDHCI_HOST_CONTROL, HostCtrl);
  }

  Status = EmmcSwitch (EMMC_EXT_CSD_HS_TIMING, EMMC_HS_TIMING_HS);
  if (!EFI_ERROR (Status)) {
    SprdSdhciSetClock (52000000);
  }

  mMedia.MediaId          = 0;
  mMedia.RemovableMedia   = FALSE;
  mMedia.MediaPresent     = TRUE;
  mMedia.LogicalPartition = FALSE;
  mMedia.ReadOnly         = FALSE;
  mMedia.WriteCaching     = FALSE;
  mMedia.BlockSize        = EMMC_BLOCK_SIZE;
  mMedia.IoAlign          = CACHE_LINE_SIZE;
  mMedia.LastBlock        = (UINT64)SecCount - 1;

  DEBUG ((DEBUG_INFO, "SprdEmmc: Init OK HC=0x%02x CLK=0x%04x Last=%lu\n",
    MmioRead8 (SDHCI_BASE + SDHCI_HOST_CONTROL),
    MmioRead16 (SDHCI_BASE + SDHCI_CLOCK_CONTROL),
    mMedia.LastBlock));

  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI
EmmcReset (
  IN EFI_BLOCK_IO_PROTOCOL *This,
  IN BOOLEAN                ExtendedVerification
  )
{
  SprdSdhciReset (SDHCI_RESET_CMD | SDHCI_RESET_DATA);
  HostRestoreAfterReset ();
  return EFI_SUCCESS;
}

STATIC EFI_STATUS
EmmcDoTransfer (
  IN     EFI_LBA   Lba,
  IN     UINTN     BufferSize,
  IN OUT VOID      *Buffer,
  IN     BOOLEAN   IsRead
  )
{
  UINT32     TotalBlocks;
  UINT32     Done;
  UINT32     Chunk;
  EFI_STATUS Status;
  UINT8      *Ptr;
  UINT16     CmdSingle;
  UINT16     CmdMulti;
  BOOLEAN    UseBounce;

  if (Buffer == NULL || BufferSize == 0) {
    return EFI_INVALID_PARAMETER;
  }
  if (BufferSize % EMMC_BLOCK_SIZE != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  TotalBlocks = (UINT32)(BufferSize / EMMC_BLOCK_SIZE);
  Ptr         = (UINT8 *)Buffer;
  Done        = 0;
  CmdSingle   = IsRead ? 17 : 24;
  CmdMulti    = IsRead ? 18 : 25;

  while (Done < TotalBlocks) {
    Chunk = TotalBlocks - Done;

    UseBounce   = (((UINTN)Ptr & (CACHE_LINE_SIZE - 1)) != 0);

    if (UseBounce) {
      if (Chunk > BOUNCE_BUF_BLOCKS) {
        Chunk = BOUNCE_BUF_BLOCKS;
      }

      if (!IsRead) {
        CopyMem (mBounceBuffer, Ptr, (UINTN)Chunk * EMMC_BLOCK_SIZE);
      }

      Status = EmmcSendDataCmd (
                 (Chunk > 1) ? CmdMulti : CmdSingle,
                 (UINT32)(Lba + Done),
                 mBounceBuffer,
                 Chunk,
                 IsRead
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (IsRead) {
        CopyMem (Ptr, mBounceBuffer, (UINTN)Chunk * EMMC_BLOCK_SIZE);
      }
    } else {
      if (Chunk > EMMC_MAX_XFER_BLOCKS) {
        Chunk = EMMC_MAX_XFER_BLOCKS;
      }

      Status = EmmcSendDataCmd (
                 (Chunk > 1) ? CmdMulti : CmdSingle,
                 (UINT32)(Lba + Done),
                 Ptr,
                 Chunk,
                 IsRead
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }

    Done += Chunk;
    Ptr  += (UINTN)Chunk * EMMC_BLOCK_SIZE;
  }

  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI
EmmcReadBlocksProto (
  IN  EFI_BLOCK_IO_PROTOCOL *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSize,
  OUT VOID                  *Buffer
  )
{
  return EmmcDoTransfer (Lba, BufferSize, Buffer, TRUE);
}

STATIC EFI_STATUS EFIAPI
EmmcWriteBlocksProto (
  IN EFI_BLOCK_IO_PROTOCOL *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  IN VOID                  *Buffer
  )
{
  return EmmcDoTransfer (Lba, BufferSize, Buffer, FALSE);
}

STATIC EFI_STATUS EFIAPI
EmmcFlushBlocks (IN EFI_BLOCK_IO_PROTOCOL *This)
{
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
SprdEmmcDxeEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  Status = SprdClockInit ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = AdmaAllocResources ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EmmcInitCardFull ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mBlockIo.Revision    = EFI_BLOCK_IO_PROTOCOL_REVISION2;
  mBlockIo.Media       = &mMedia;
  mBlockIo.Reset       = EmmcReset;
  mBlockIo.ReadBlocks  = EmmcReadBlocksProto;
  mBlockIo.WriteBlocks = EmmcWriteBlocksProto;
  mBlockIo.FlushBlocks = EmmcFlushBlocks;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mDevicePath,
                  &gEfiBlockIoProtocolGuid,
                  &mBlockIo,
                  NULL
                  );

  return Status;
}