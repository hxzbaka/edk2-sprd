// SPDX-License-Identifier: BSD-2-Clause-Patent
#include <Uefi.h>
#include <Guid/EventGroup.h>
#include <Library/BaseLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/PartitionInfo.h>

#define PERSISTENT_RAM_SIGNATURE  0x43474244U
#define PERSISTENT_RAM_HEADER_SIZE  12
#define EMBEDDED_PANIC_MAGIC  0x46554E4BU
#define EMBEDDED_PANIC_VERSION  2

#pragma pack(1)
typedef struct {
  UINT32 Magic;
  UINT32 Crc;
  UINT32 Version;
  UINT64 Flags;
  UINT32 PanicOffset;
  UINT32 PanicLength;
  UINT32 StackshotOffset;
  UINT32 StackshotLength;
  UINT32 OtherOffset;
  UINT32 OtherLength;
  UINT64 PowerState;
  CHAR8 OsVersion[32];
  CHAR8 MacOsVersion[32];
} EMBEDDED_PANIC_HEADER;
#pragma pack()

STATIC_ASSERT (sizeof (EMBEDDED_PANIC_HEADER) == 116, "XNU embedded panic header ABI");

STATIC CONST CHAR8 mPendingPrefix[] = "====0.0-D\nXNU panic: recovery=pending\n";
STATIC CONST CHAR8 mWrittenState[] = "written";

STATIC VOID EFIAPI
SavePanicLog (EFI_EVENT Event, VOID *Context)
{
  UINT64 Base;
  UINT32 RecordSize;
  UINT32 RegionSize;
  UINT32 Crc;
  BOOLEAN Native;
  EMBEDDED_PANIC_HEADER NativeHeader;
  UINT32 Start;
  UINT32 Size;
  volatile UINT32 *Header;
  volatile UINT8 *Payload;
  EFI_HANDLE *Handles;
  EFI_PARTITION_INFO_PROTOCOL *Partition;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  EFI_BLOCK_IO_PROTOCOL *Candidate;
  EFI_STATUS Status;
  UINTN Count;
  UINTN Index;
  UINTN Matches;
  UINTN NameLength;
  UINTN Pages;
  UINTN Bytes;
  UINTN BlockCount;
  UINT64 LogOffset;
  EFI_LBA LogLba;
  VOID *Buffer;
  CONST CHAR16 *Name;

  (VOID)Context;
  gBS->CloseEvent (Event);
  Base = FixedPcdGet64 (PcdDarwinRamoopsBase);
  RegionSize = FixedPcdGet32 (PcdDarwinRamoopsSize);
  RecordSize = FixedPcdGet32 (PcdDarwinRamoopsRecordSize);
  if ((Base == 0) || ((Base & (sizeof (UINT32) - 1)) != 0) ||
      (RecordSize > RegionSize) ||
      (RecordSize < PERSISTENT_RAM_HEADER_SIZE + sizeof (mPendingPrefix) - 1)) {
    Status = EFI_INVALID_PARAMETER;
    goto Error;
  }
  Header = (volatile UINT32 *)(UINTN)Base;
  Native = (Header[0] == EMBEDDED_PANIC_MAGIC);
  if (Native) {
    if (RegionSize < sizeof (NativeHeader)) { Status = EFI_BAD_BUFFER_SIZE; goto Error; }
    CopyMem (&NativeHeader, (CONST VOID *)(UINTN)Base, sizeof (NativeHeader));
    if ((NativeHeader.Version != EMBEDDED_PANIC_VERSION) || (NativeHeader.PanicLength == 0) ||
        (NativeHeader.PanicOffset < sizeof (NativeHeader)) ||
        (NativeHeader.PanicOffset > RegionSize) ||
        (NativeHeader.PanicLength > RegionSize - NativeHeader.PanicOffset) ||
        ((NativeHeader.StackshotLength != 0) &&
         ((NativeHeader.StackshotOffset < sizeof (NativeHeader)) ||
          (NativeHeader.StackshotOffset > RegionSize) ||
          (NativeHeader.StackshotLength > RegionSize - NativeHeader.StackshotOffset))) ||
        ((NativeHeader.OtherLength != 0) &&
         ((NativeHeader.OtherOffset < sizeof (NativeHeader)) ||
          (NativeHeader.OtherOffset > RegionSize) ||
          (NativeHeader.OtherLength > RegionSize - NativeHeader.OtherOffset)))) {
      Status = EFI_COMPROMISED_DATA;
      goto Error;
    }
    Status = gBS->CalculateCrc32 ((VOID *)(UINTN)(Base + OFFSET_OF (EMBEDDED_PANIC_HEADER, Version)),
                                RegionSize - OFFSET_OF (EMBEDDED_PANIC_HEADER, Version), &Crc);
    if (EFI_ERROR (Status)) { goto Error; }
    if (Crc != NativeHeader.Crc) { Status = EFI_CRC_ERROR; goto Error; }
    Payload = (volatile UINT8 *)(UINTN)Base;
    RecordSize = RegionSize;
    Size = RegionSize;
  } else {
    Start = Header[1];
    Size = Header[2];
    if ((Header[0] != PERSISTENT_RAM_SIGNATURE) || (Start != Size) ||
        (Size < sizeof (mPendingPrefix) - 1) || (Size > RecordSize - PERSISTENT_RAM_HEADER_SIZE)) {
      return;
    }
    Payload = (volatile UINT8 *)(UINTN)(Base + PERSISTENT_RAM_HEADER_SIZE);
    for (Index = 0; Index < sizeof (mPendingPrefix) - 1; Index++) {
      if (Payload[Index] != (UINT8)mPendingPrefix[Index]) { return; }
    }
  }

  Name = (CONST CHAR16 *)FixedPcdGetPtr (PcdDarwinPanicLogPartitionName);
  NameLength = StrLen (Name);
  if (NameLength == 0) { Status = EFI_INVALID_PARAMETER; goto Error; }
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPartitionInfoProtocolGuid, NULL, &Count, &Handles);
  if (EFI_ERROR (Status)) { goto Error; }
  Candidate = NULL;
  Matches = 0;
  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &gEfiPartitionInfoProtocolGuid, (VOID **)&Partition);
    if (EFI_ERROR (Status) || (Partition->Type != PARTITION_TYPE_GPT) ||
        (NameLength >= ARRAY_SIZE (Partition->Info.Gpt.PartitionName)) ||
        (CompareMem (Partition->Info.Gpt.PartitionName, Name, (NameLength + 1) * sizeof (CHAR16)) != 0)) {
      continue;
    }
    Matches++;
    Status = gBS->HandleProtocol (Handles[Index], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo);
    if (!EFI_ERROR (Status)) { Candidate = BlockIo; }
  }
  FreePool (Handles);
  if (Matches != 1) {
    Status = (Matches == 0) ? EFI_NOT_FOUND : EFI_NO_MAPPING;
    goto Error;
  }
  if ((Candidate == NULL) || (Candidate->Media == NULL) || !Candidate->Media->LogicalPartition ||
      !Candidate->Media->MediaPresent || Candidate->Media->ReadOnly ||
      (Candidate->Media->BlockSize == 0)) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }
  LogOffset = FixedPcdGet64 (PcdDarwinPanicLogOffset);
  if (LogOffset % Candidate->Media->BlockSize != 0) {
    Status = EFI_INVALID_PARAMETER;
    goto Error;
  }
  LogLba = LogOffset / Candidate->Media->BlockSize;
  BlockCount = ((UINTN)RecordSize + Candidate->Media->BlockSize - 1) / Candidate->Media->BlockSize;
  if ((LogLba > Candidate->Media->LastBlock) ||
      (BlockCount - 1 > Candidate->Media->LastBlock - LogLba)) {
    Status = EFI_VOLUME_FULL;
    goto Error;
  }
  Bytes = BlockCount * Candidate->Media->BlockSize;
  Pages = EFI_SIZE_TO_PAGES (Bytes);
  Buffer = AllocateAlignedPages (Pages, MAX (EFI_PAGE_SIZE, Candidate->Media->IoAlign));
  if (Buffer == NULL) { Status = EFI_OUT_OF_RESOURCES; goto Error; }
  Status = Candidate->ReadBlocks (Candidate, Candidate->Media->MediaId, LogLba, Bytes, Buffer);
  if (!EFI_ERROR (Status)) {
    ZeroMem (Buffer, RecordSize);
    for (Index = 0; Index < Size; Index++) {
      ((UINT8 *)Buffer)[Index] = Payload[Index];
    }
    Status = Candidate->WriteBlocks (Candidate, Candidate->Media->MediaId, LogLba, Bytes, Buffer);
    if (!EFI_ERROR (Status)) { Status = Candidate->FlushBlocks (Candidate); }
  }
  FreeAlignedPages (Buffer, Pages);
  if (EFI_ERROR (Status)) { goto Error; }

  // Consume the RAM record only after the log has been flushed to storage.
  if (Native) {
    Header[0] = 0;
  } else {
    Index = sizeof (mPendingPrefix) - sizeof ("pending") - 1;
    for (UINTN Offset = 0; Offset < sizeof (mWrittenState) - 1; Offset++) {
      Payload[Index + Offset] = (UINT8)mWrittenState[Offset];
    }
  }
  WriteBackDataCacheRange ((VOID *)(UINTN)Base, RecordSize);
  ArmDataSynchronizationBarrier ();
  Print (L"Darwin panic log saved: %s offset 0x%lx, %u bytes\n", Name, LogOffset, Size);
  DEBUG ((DEBUG_ERROR, "Darwin panic log saved: %s offset 0x%lx, %u bytes\n", Name, LogOffset, Size));
  return;
Error:
  Print (L"Darwin panic log save failed: %r; pending record retained\n", Status);
  DEBUG ((DEBUG_ERROR, "Darwin panic log save failed: %r; pending record retained\n", Status));
}

EFI_STATUS EFIAPI
DarwinPanicRecoveryEntry (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_EVENT Event;
  (VOID)ImageHandle;
  (VOID)SystemTable;
  return gBS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, SavePanicLog, NULL,
                           &gEfiEventReadyToBootGuid, &Event);
}
