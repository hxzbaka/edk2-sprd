// SPDX-License-Identifier: BSD-2-Clause-Patent
#include <Uefi.h>
#include <Guid/FileInfo.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/RngLib.h>
#include <Library/UefiLib.h>
#include <Protocol/UniXNULoader.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include "BootArgs.h"
#include "BootVideo.h"
#include "PackedRamdisk.h"
#include "WindowSize.h"

#define BLOCK_SIZE   SIZE_2MB
#define MAX_SEGMENTS 64
#define OUTSIDE_MEMORY_RANGE_BYTES (2 * sizeof (UINT64))

typedef struct {
  UINT64  Address;
  UINT64  Size;
  UINT64  FileOffset;
  UINT64  FileSize;
  UINT32  Protection;
} SEGMENT;

typedef struct {
  SEGMENT  Segments[MAX_SEGMENTS];
  UINTN    Count;
  UINT64   Base;
  UINT64   End;
  UINT64   Entry;
  BOOLEAN  Arm32;
} KERNEL;

STATIC EFI_HANDLE  mImage;
UINT64 EFIAPI DarwinCurrentEl (VOID);
VOID EFIAPI DarwinEnter (UINT64 Args, UINT64 Entry, UINT64 Memory, UINT64 Size, UINT64 VideoBase, UINT64 VideoBytes, BOOLEAN Arm32);

#define READ_SYSREG(Name)  ({ UINT64 Value_; __asm__ volatile ("mrs %0, " #Name : "=r" (Value_)); Value_; })

STATIC UINT32 U32 (CONST VOID *P) { return ReadUnaligned32 ((CONST UINT32 *)P); }
STATIC UINT64 U64 (CONST VOID *P) { return ReadUnaligned64 ((CONST UINT64 *)P); }

STATIC
EFI_STATUS
ReadBootVideo (UINT64 Video[6])
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL           *Gop;
  EFI_STATUS                            Status;
  EFI_STATUS                            VideoStatus;
  EFI_HANDLE                            *Handles;
  UINTN                                 Count;
  UINTN                                 Index;

  ZeroMem (Video, 6 * sizeof (*Video));
  Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **)&Gop);
  VideoStatus = EFI_SUCCESS;
  if (!EFI_ERROR (Status)) {
    VideoStatus = DarwinBootVideoFromGop (Gop, Video);
    if (!EFI_ERROR (VideoStatus)) { return VideoStatus; }
  }
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &Count, &Handles);
  if (Status == EFI_NOT_FOUND) { return VideoStatus; }
  if (EFI_ERROR (Status)) { return Status; }
  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &gEfiGraphicsOutputProtocolGuid, (VOID **)&Gop);
    if (EFI_ERROR (Status)) { VideoStatus = Status; continue; }
    VideoStatus = DarwinBootVideoFromGop (Gop, Video);
    if (!EFI_ERROR (VideoStatus)) { break; }
  }
  FreePool (Handles);
  return VideoStatus;
}

STATIC
EFI_STATUS
ReadFile (
  EFI_FILE_PROTOCOL *Root,
  CONST CHAR16      *Path,
  UINTN             Limit,
  VOID              **Data,
  UINTN             *Size
  )
{
  EFI_STATUS         Status;
  EFI_FILE_PROTOCOL  *File;
  EFI_FILE_INFO      *Info;
  UINTN              InfoSize;
  UINTN              Actual;

  *Data = NULL;
  *Size = 0;
  Status = Root->Open (Root, &File, (CHAR16 *)Path, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) { return Status; }
  Info = NULL;
  InfoSize = 0;
  Status = File->GetInfo (File, &gEfiFileInfoGuid, &InfoSize, NULL);
  if (Status != EFI_BUFFER_TOO_SMALL) { goto Done; }
  Info = AllocatePool (InfoSize);
  if (Info == NULL) { Status = EFI_OUT_OF_RESOURCES; goto Done; }
  Status = File->GetInfo (File, &gEfiFileInfoGuid, &InfoSize, Info);
  if (EFI_ERROR (Status)) { goto Done; }
  if ((Info->Attribute & EFI_FILE_DIRECTORY) || (Info->FileSize == 0) || (Info->FileSize > Limit)) {
    Print (L"DARWINBOOT: %s attr %lx size %lu limit %lu\n", Path, (UINT64)Info->Attribute, (UINT64)Info->FileSize, (UINT64)Limit);
    Status = EFI_BAD_BUFFER_SIZE;
    goto Done;
  }
  *Size = (UINTN)Info->FileSize;
  *Data = AllocatePool (*Size);
  if (*Data == NULL) { Status = EFI_OUT_OF_RESOURCES; goto Done; }
  Actual = *Size;
  Status = File->Read (File, &Actual, *Data);
  if (!EFI_ERROR (Status) && (Actual != *Size)) { Status = EFI_DEVICE_ERROR; }
Done:
  if (Info != NULL) { FreePool (Info); }
  File->Close (File);
  if (EFI_ERROR (Status) && (*Data != NULL)) { FreePool (*Data); *Data = NULL; }
  return Status;
}

STATIC
EFI_STATUS
ParseKernel (CONST UINT8 *Data, UINTN Size, KERNEL *Kernel)
{
  UINTN    Offset;
  UINTN    End;
  UINTN    Index;
  UINTN    Other;
  UINT32   Count;
  UINT32   Command;
  UINT32   Length;
  SEGMENT  *Segment;
  BOOLEAN  HasEntry;
  BOOLEAN  Executable;

  if (Size < 28) { return EFI_LOAD_ERROR; }
  ZeroMem (Kernel, sizeof (*Kernel));
  Kernel->Arm32 = U32 (Data) == 0xfeedface;
  if ((!Kernel->Arm32 && ((Size < 32) || (U32 (Data) != 0xfeedfacf) ||
       (U32 (Data + 4) != 0x100000c) || (U32 (Data + 8) != 0))) ||
      (Kernel->Arm32 && ((U32 (Data + 4) != 12) || (U32 (Data + 8) != 12))) ||
      (U32 (Data + 12) != 2)) { return EFI_UNSUPPORTED; }
  Offset = Kernel->Arm32 ? 28 : 32;
  if (U32 (Data + 20) > Size - Offset) { return EFI_LOAD_ERROR; }
  Count = U32 (Data + 16);
  End = Offset + U32 (Data + 20);
  HasEntry = FALSE;
  for (Index = 0; Index < Count; Index++) {
    if (End - Offset < 8) { return EFI_LOAD_ERROR; }
    Command = U32 (Data + Offset);
    Length = U32 (Data + Offset + 4);
    if ((Length < 8) || (Length & (Kernel->Arm32 ? 3 : 7)) || (Length > End - Offset)) { return EFI_LOAD_ERROR; }
    if (Kernel->Arm32 && (Command == 1)) {
      if ((Length < 56) || (U32 (Data + Offset + 48) > (Length - 56) / 68) ||
          (Kernel->Count == MAX_SEGMENTS)) { return EFI_LOAD_ERROR; }
      if (U32 (Data + Offset + 28) != 0) {
        Segment = &Kernel->Segments[Kernel->Count++];
        Segment->Address = U32 (Data + Offset + 24);
        Segment->Size = U32 (Data + Offset + 28);
        Segment->FileOffset = U32 (Data + Offset + 32);
        Segment->FileSize = U32 (Data + Offset + 36);
        Segment->Protection = U32 (Data + Offset + 44);
        if ((Segment->Size > FixedPcdGet64 (PcdXnuMaxKernelSize)) ||
            (Segment->Size > MAX_UINT32 - Segment->Address) ||
            (Segment->FileSize > Segment->Size) || (Segment->FileOffset > Size) ||
            (Segment->FileSize > Size - Segment->FileOffset)) { return EFI_LOAD_ERROR; }
      }
    } else if (!Kernel->Arm32 && (Command == 0x19)) {
      if ((Length < 72) || (U32 (Data + Offset + 64) > (Length - 72) / 80) ||
          (Kernel->Count == MAX_SEGMENTS)) { return EFI_LOAD_ERROR; }
      if (U64 (Data + Offset + 32) != 0) {
        Segment = &Kernel->Segments[Kernel->Count++];
        Segment->Address = U64 (Data + Offset + 24);
        Segment->Size = U64 (Data + Offset + 32);
        Segment->FileOffset = U64 (Data + Offset + 40);
        Segment->FileSize = U64 (Data + Offset + 48);
        Segment->Protection = U32 (Data + Offset + 60);
        if ((Segment->Size > FixedPcdGet64 (PcdXnuMaxKernelSize)) || (Segment->Address > MAX_UINT64 - Segment->Size) ||
            (Segment->FileSize > Segment->Size) || (Segment->FileOffset > Size) ||
            (Segment->FileSize > Size - Segment->FileOffset)) { return EFI_LOAD_ERROR; }
      }
    } else if (Command == 5) {
      if (HasEntry) { return EFI_LOAD_ERROR; }
      if (Kernel->Arm32) {
        if ((Length != 84) || (U32 (Data + Offset + 8) != 1) ||
            (U32 (Data + Offset + 12) != 17) || (U32 (Data + Offset + 80) & 0x20)) {
          return EFI_UNSUPPORTED;
        }
        Kernel->Entry = U32 (Data + Offset + 76);
      } else {
        if ((Length != 288) || (U32 (Data + Offset + 8) != 6) ||
            (U32 (Data + Offset + 12) != 68)) { return EFI_LOAD_ERROR; }
        Kernel->Entry = U64 (Data + Offset + 272);
      }
      HasEntry = TRUE;
    }
    Offset += Length;
  }
  if ((Offset != End) || !HasEntry || (Kernel->Count == 0)) { return EFI_LOAD_ERROR; }
  Kernel->Base = MAX_UINT64;
  Executable = FALSE;
  for (Index = 0; Index < Kernel->Count; Index++) {
    Segment = &Kernel->Segments[Index];
    Kernel->Base = MIN (Kernel->Base, Segment->Address);
    Kernel->End = MAX (Kernel->End, Segment->Address + Segment->Size);
    if ((Kernel->Entry >= Segment->Address) && (Kernel->Entry - Segment->Address < Segment->FileSize) &&
        ((Segment->Protection & 4) != 0)) { Executable = TRUE; }
    for (Other = 0; Other < Index; Other++) {
      if ((Segment->Address < Kernel->Segments[Other].Address + Kernel->Segments[Other].Size) &&
          (Kernel->Segments[Other].Address < Segment->Address + Segment->Size)) { return EFI_LOAD_ERROR; }
    }
  }
  if (!Executable || (Kernel->Entry & 3) || (Kernel->Base & EFI_PAGE_MASK) ||
      (FixedPcdGet64 (PcdXnuMaxKernelSize) < SIZE_2MB) ||
      (Kernel->Base > MAX_UINT64 - FixedPcdGet64 (PcdXnuMaxKernelSize)) ||
      (Kernel->End - Kernel->Base > FixedPcdGet64 (PcdXnuMaxKernelSize) - SIZE_2MB)) { return EFI_LOAD_ERROR; }
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
AdtNode (CONST UINT8 *Data, UINTN Size, UINTN *Offset, UINTN Depth, UINTN ParentPath, UINTN *RamdiskOffset, UINTN *TrustCacheOffset, UINT64 *WindowSize, BOOLEAN *KeepPacked, BOOLEAN Arm32, UINTN *SeedOffset, UINTN *OutsideOffset, UINTN *OutsideBytes)
{
  UINT32 Properties;
  UINT32 Children;
  UINT32 Length;
  UINTN  Index;
  UINTN  Path;
  UINTN  Candidate;
  UINTN  TrustCandidate;
  UINTN  MemoryCandidate;
  UINTN  OutsideCandidate;
  UINTN  OutsideLength;
  UINTN  SeedCandidate;
  UINTN  ManifestHashCandidate;
  CONST UINT8 *Name;
  UINTN  NameLength;

  Name = NULL;
  NameLength = 0;
  Candidate = MAX_UINTN;
  TrustCandidate = MAX_UINTN;
  MemoryCandidate = MAX_UINTN;
  OutsideCandidate = MAX_UINTN;
  OutsideLength = 0;
  SeedCandidate = MAX_UINTN;
  ManifestHashCandidate = MAX_UINTN;
  if ((Depth > 64) || (Size - *Offset < 8)) { return FALSE; }
  Properties = U32 (Data + *Offset);
  Children = U32 (Data + *Offset + 4);
  *Offset += 8;
  if (Properties > (Size - *Offset) / 36) { return FALSE; }
  for (Index = 0; Index < Properties; Index++) {
    if (Size - *Offset < 36) { return FALSE; }
    if (ScanMem8 (Data + *Offset, 32, 0) == NULL) { return FALSE; }
    Length = U32 (Data + *Offset + 32);
    *Offset += 36;
    if (Length > Size - *Offset) { return FALSE; }
    if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "name") == 0) {
      if (Name != NULL) { return FALSE; }
      Name = Data + *Offset;
      NameLength = Length;
    }
    if ((AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "RAMDisk") == 0) ||
        (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "PackedRAMDisk") == 0)) {
      if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "PackedRAMDisk") == 0) {
        if (*KeepPacked) { return FALSE; }
        *KeepPacked = TRUE;
      }
      if ((Candidate != MAX_UINTN) || (Length != (Arm32 ? 8 : 16))) { return FALSE; }
      Candidate = *Offset;
    }
    if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "TrustCache") == 0) {
      if ((TrustCandidate != MAX_UINTN) || (Length != (Arm32 ? 8 : 16))) { return FALSE; }
      TrustCandidate = *Offset;
    }
    if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "random-seed") == 0) {
      if ((SeedCandidate != MAX_UINTN) || (Length != 64)) { return FALSE; }
      SeedCandidate = *Offset;
    }
    if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "boot-manifest-hash") == 0) {
      if ((ManifestHashCandidate != MAX_UINTN) ||
          ((Length != 20) && (Length != 32) && (Length != 48)) ||
          IsZeroBuffer (Data + *Offset, Length)) { return FALSE; }
      ManifestHashCandidate = *Offset;
    }
    if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "darwinboot-memory-size") == 0) {
      if ((MemoryCandidate != MAX_UINTN) || (Length != 8)) { return FALSE; }
      MemoryCandidate = *Offset;
    }
    if (AsciiStrCmp ((CONST CHAR8 *)(Data + *Offset - 36), "darwinboot-memory-outside") == 0) {
      if ((OutsideCandidate != MAX_UINTN) || (Length < OUTSIDE_MEMORY_RANGE_BYTES) ||
          ((Length % OUTSIDE_MEMORY_RANGE_BYTES) != 0) ||
          !IsZeroBuffer (Data + *Offset, Length)) { return FALSE; }
      OutsideCandidate = *Offset;
      OutsideLength = Length;
    }
    *Offset += Length;
    if (ALIGN_VALUE (*Offset, 4) > Size) { return FALSE; }
    *Offset = ALIGN_VALUE (*Offset, 4);
  }
  Path = 3;
  if (Depth == 0) { Path = 0; }
  if ((Depth == 1) && (ParentPath == 0) && (NameLength == 7) && (CompareMem (Name, "chosen", 7) == 0)) { Path = 1; }
  if ((Depth == 2) && (ParentPath == 1) && (NameLength == 11) && (CompareMem (Name, "memory-map", 11) == 0)) { Path = 2; }
  if ((Path == 2) && (Candidate != MAX_UINTN)) {
    if (*RamdiskOffset != MAX_UINTN) { return FALSE; }
    *RamdiskOffset = Candidate;
  }
  if ((Path == 2) && (TrustCandidate != MAX_UINTN)) {
    if (*TrustCacheOffset != MAX_UINTN) { return FALSE; }
    *TrustCacheOffset = TrustCandidate;
  }
  if ((Path == 1) && (SeedCandidate != MAX_UINTN)) {
    if (*SeedOffset != MAX_UINTN) { return FALSE; }
    *SeedOffset = SeedCandidate;
  }
  if (MemoryCandidate != MAX_UINTN) {
    if ((Path != 1) || (*WindowSize != 0)) { return FALSE; }
    *WindowSize = U64 (Data + MemoryCandidate);
    if (!DarwinWindowSizeValid (*WindowSize, FixedPcdGet64 (PcdXnuMinWindowSize), FixedPcdGet64 (PcdXnuMaxWindowSize))) { return FALSE; }
  }
  if (OutsideCandidate != MAX_UINTN) {
    if ((Path != 1) || (*OutsideOffset != MAX_UINTN)) { return FALSE; }
    *OutsideOffset = OutsideCandidate;
    *OutsideBytes = OutsideLength;
  }
  if (Children > (Size - *Offset) / 8) { return FALSE; }
  for (Index = 0; Index < Children; Index++) {
    if (!AdtNode (Data, Size, Offset, Depth + 1, Path, RamdiskOffset, TrustCacheOffset, WindowSize, KeepPacked, Arm32, SeedOffset, OutsideOffset, OutsideBytes)) { return FALSE; }
  }
  return TRUE;
}

STATIC
BOOLEAN
ValidTrustCaches (CONST UINT8 *Data, UINTN Size)
{
  UINTN Count;
  UINTN Index;
  UINTN Start;
  UINTN End;
  UINTN Entries;
  UINTN Entry;
  UINTN EntrySize;
  UINT32 Version;
  INTN HashOrder;

  if (Size < 8) { return FALSE; }
  Count = U32 (Data);
  if ((Count == 0) || (Count > 128) || (Count > (Size - 4) / 4)) { return FALSE; }
  for (Index = 0; Index < Count; Index++) {
    Start = U32 (Data + 4 + Index * 4);
    End = (Index + 1 < Count) ? U32 (Data + 8 + Index * 4) : Size;
    if ((Start < 4 + Count * 4) || (Start & 3) || (End > Size) ||
        (End < Start) || (End - Start < 24)) { return FALSE; }
    Version = U32 (Data + Start);
    if ((Version != 1) && (Version != 2)) { return FALSE; }
    EntrySize = (Version == 1) ? 22 : 24;
    Entries = U32 (Data + Start + 20);
    if ((Entries > (End - Start - 24) / EntrySize) || (End - Start != 24 + Entries * EntrySize)) { return FALSE; }
    for (Entry = 0; Entry < Entries; Entry++) {
      CONST UINT8 *Hash = Data + Start + 24 + Entry * EntrySize;
      HashOrder = (Entry == 0) ? -1 : CompareMem (Hash - EntrySize, Hash, 20);
      if ((HashOrder > 0) || ((Version == 2) && ((Hash[20] < 1) || (Hash[20] > 4) || (HashOrder == 0)))) { return FALSE; }
    }
  }
  return TRUE;
}

STATIC
EFI_STATUS
GetMap (EFI_MEMORY_DESCRIPTOR **Map, UINTN *Capacity, UINTN *Size, UINTN *Key, UINTN *Stride)
{
  EFI_STATUS Status;
  UINT32     Version;
  UINTN      Attempt;

  for (Attempt = 0; Attempt < 3; Attempt++) {
    *Size = *Capacity;
    Status = gBS->GetMemoryMap (Size, *Map, Key, Stride, &Version);
    if (Status != EFI_BUFFER_TOO_SMALL) { return Status; }
    if (*Map != NULL) { FreePool (*Map); }
    *Capacity = *Size + 32 * sizeof (EFI_MEMORY_DESCRIPTOR);
    *Map = AllocatePool (*Capacity);
    if (*Map == NULL) { return EFI_OUT_OF_RESOURCES; }
  }
  return EFI_OUT_OF_RESOURCES;
}

// End of the run of EfiConventionalMemory that begins at Start: the UEFI map often lists physically
// adjacent free ranges as separate descriptors, so extend over every descriptor that continues the run.
STATIC
UINT64
RunEnd (EFI_MEMORY_DESCRIPTOR *Map, UINTN Size, UINTN Stride, UINT64 Start, UINT64 End)
{
  EFI_MEMORY_DESCRIPTOR *Descriptor;
  UINTN                 Offset;
  BOOLEAN               Extended;

  do {
    Extended = FALSE;
    for (Offset = 0; Offset + Stride <= Size; Offset += Stride) {
      Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + Offset);
      if ((Descriptor->Type == EfiConventionalMemory) && (Descriptor->PhysicalStart == End) &&
          (Descriptor->NumberOfPages != 0) && (Descriptor->NumberOfPages < (MAX_UINT64 - End) / EFI_PAGE_SIZE)) {
        End += EFI_PAGES_TO_SIZE (Descriptor->NumberOfPages);
        Extended = TRUE;
      }
    }
  } while (Extended);
  (VOID)Start;
  return End;
}

// Conventional descriptors in the final map exclude the allocated boot window.
// Preserve firmware reservations and emit ascending disjoint physical runs.
STATIC
EFI_STATUS
DescribeOutsideMemory (EFI_MEMORY_DESCRIPTOR *Map, UINTN Size, UINTN Stride, UINT8 *Table, UINTN TableBytes)
{
  EFI_MEMORY_DESCRIPTOR *Descriptor;
  EFI_MEMORY_DESCRIPTOR *Next;
  UINT64                 Cursor;
  UINT64                 End;
  UINTN                  Offset;

  ZeroMem (Table, TableBytes);
  Cursor = 0;
  for (;;) {
    Next = NULL;
    for (Offset = 0; Offset + Stride <= Size; Offset += Stride) {
      Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + Offset);
      if ((Descriptor->Type == EfiConventionalMemory) &&
          (Descriptor->NumberOfPages != 0) && (Descriptor->PhysicalStart >= Cursor) &&
          ((Next == NULL) || (Descriptor->PhysicalStart < Next->PhysicalStart))) {
        Next = Descriptor;
      }
    }
    if (Next == NULL) { return EFI_SUCCESS; }
    if (TableBytes < OUTSIDE_MEMORY_RANGE_BYTES) { return EFI_BUFFER_TOO_SMALL; }
    if (Next->NumberOfPages > (MAX_UINT64 - Next->PhysicalStart) / EFI_PAGE_SIZE) {
      return EFI_COMPROMISED_DATA;
    }
    End = RunEnd (Map, Size, Stride, Next->PhysicalStart,
                  Next->PhysicalStart + EFI_PAGES_TO_SIZE (Next->NumberOfPages));
    WriteUnaligned64 ((UINT64 *)Table, Next->PhysicalStart);
    WriteUnaligned64 ((UINT64 *)(Table + sizeof (UINT64)), End - Next->PhysicalStart);
    Table += OUTSIDE_MEMORY_RANGE_BYTES;
    TableBytes -= OUTSIDE_MEMORY_RANGE_BYTES;
    Cursor = End;
  }
}

// Large diagnostic text straight into the GOP framebuffer: the UEFI text console font is too small to
// photograph. 5x12 glyphs from DiagFont.h scaled 4x, white on a black box; wraps at the right edge.
#include "DiagFont.h"
STATIC CONST UINT64 mDiagFont[] = GLYPH_FONT;

STATIC
VOID
DiagDraw (CONST CHAR16 *Text, UINTN Row)
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  UINT64                       Video[6];
  UINTN                        Scale = 4, X = 0, Y, Index, Gx, Gy, Dx, Dy, Cell, Line;
  UINT32                       *Fb;
  UINT64                       Glyph, Data;
  CHAR16                       Ch;

  if (EFI_ERROR (gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&Gop)) ||
      EFI_ERROR (DarwinBootVideoFromGop (Gop, Video))) { return; }
  Cell = 6 * Scale;
  Line = 14 * Scale;
  Y = Video[4] / 4 + Row * Line;
  Fb = (UINT32 *)(UINTN)Video[0];
  for (Index = 0; Text[Index] != 0; Index++) {
    Ch = Text[Index];
    if ((Ch < 32) || (Ch > 126)) { continue; }
    if (X + Cell > Video[3]) { X = 0; Y += Line; }
    if (Y + Line > Video[4]) { return; }
    Glyph = mDiagFont[Ch - 32];
    for (Gy = 0; Gy < 14 * Scale; Gy++) {
      for (Gx = 0; Gx < Cell; Gx++) {
        Fb[(Y + Gy) * (Video[2] / 4) + X + Gx] = 0xFF000000;
      }
    }
    for (Gy = 0; Gy < 12; Gy++) {
      Data = (Gy < 6) ? ((Glyph >> 32) >> (Gy * 5)) : ((Glyph & 0xFFFFFFFFULL) >> ((Gy - 6) * 5));
      for (Gx = 0; Gx < 5; Gx++) {
        if ((Data >> Gx) & 1) {
          for (Dy = 0; Dy < Scale; Dy++) {
            for (Dx = 0; Dx < Scale; Dx++) {
              Fb[(Y + Gy * Scale + Dy) * (Video[2] / 4) + X + Gx * Scale + Dx] = 0xFFFFFFFF;
            }
          }
        }
      }
    }
    X += Cell;
  }
}

STATIC
EFI_STATUS
AllocateWindow (KERNEL *Kernel, UINT64 WindowSize, EFI_PHYSICAL_ADDRESS *Address)
{
  EFI_MEMORY_DESCRIPTOR *Map;
  EFI_MEMORY_DESCRIPTOR *Descriptor;
  EFI_STATUS            Status;
  UINTN                 Capacity;
  UINTN                 Size;
  UINTN                 Key;
  UINTN                 Stride;
  UINTN                 Offset;
  UINT64                End;
  UINT64                Candidate;
  UINT64                BestStart;
  UINT64                BestBytes;
  UINT64                RunBytes;

  *Address = 0;
  Map = NULL;
  Capacity = 0;
  BestStart = 0;
  BestBytes = 0;
  Status = GetMap (&Map, &Capacity, &Size, &Key, &Stride);
  if (EFI_ERROR (Status)) { if (Map != NULL) { FreePool (Map); } return Status; }
  Status = EFI_OUT_OF_RESOURCES;
  for (Offset = 0; Offset + Stride <= Size; Offset += Stride) {
    Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + Offset);
    if ((Descriptor->Type != EfiConventionalMemory) || (Descriptor->NumberOfPages == 0)) { continue; }
    End = RunEnd (Map, Size, Stride, Descriptor->PhysicalStart,
                  Descriptor->PhysicalStart + EFI_PAGES_TO_SIZE (Descriptor->NumberOfPages));
    RunBytes = End - Descriptor->PhysicalStart;
    if (RunBytes > BestBytes) { BestBytes = RunBytes; BestStart = Descriptor->PhysicalStart; }
    Candidate = ALIGN_VALUE (Descriptor->PhysicalStart, BLOCK_SIZE) + (Kernel->Arm32 ? 0 : (Kernel->Base & (BLOCK_SIZE - 1)));
    if ((Candidate < Descriptor->PhysicalStart) || (End < Candidate) || (End - Candidate < WindowSize)) { continue; }
    if (Kernel->Arm32 && ((Candidate > MAX_UINT32) || (WindowSize > MAX_UINT32 - Candidate))) { continue; }
    Status = gBS->AllocatePages (AllocateAddress, EfiLoaderCode, EFI_SIZE_TO_PAGES (WindowSize), &Candidate);
    if (!EFI_ERROR (Status)) { *Address = Candidate; break; }
  }
  if (EFI_ERROR (Status)) {
    CHAR16 Line[96];
    UINTN  Row = 1;
    UnicodeSPrint (Line, sizeof (Line), L"NO WINDOW %lx BIG RUN %lx @ %lx", WindowSize, BestBytes, BestStart);
    DiagDraw (Line, 0);
    for (Offset = 0; Offset + Stride <= Size; Offset += Stride) {
      Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + Offset);
      if ((Descriptor->Type == EfiConventionalMemory) && (Descriptor->NumberOfPages >= 0x400) && (Row < 8)) {
        UnicodeSPrint (Line, sizeof (Line), L"FREE %lx + %lx", Descriptor->PhysicalStart, EFI_PAGES_TO_SIZE (Descriptor->NumberOfPages));
        DiagDraw (Line, Row++);
      }
    }

  }
  FreePool (Map);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Boot (
  IN UNIXNU_LOADER_PROTOCOL    *This,
  IN CONST UNIXNU_LOADER_BOOT_REQUEST *Request
  )
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_FILE_PROTOCOL               *Root;
  VOID                            *Image;
  VOID                            *Tree;
  VOID                            *TrustCaches;
  UINTN                           ImageSize;
  UINTN                           TreeSize;
  UINTN                           TrustCacheSize;
  CHAR16                          *TreePath;
  UINTN                           Length;
  UINTN                           Index;
  UINTN                           Offset;
  KERNEL                          Kernel;
  EFI_PHYSICAL_ADDRESS            Address;
  UINT64                          ArgsAddress;
  UINT64                          TreeAddress;
  UINT64                          Used;
  UINT64                          WindowSize;
  UINT64                          RamdiskAddress;
  UINTN                           RamdiskOffset;
  UINTN                           RamdiskBytes;
  UINTN                           RamdiskExpandedSize;
  BOOLEAN                         RamdiskPacked;
  BOOLEAN                         KeepPacked;
  UINTN                           TrustCacheOffset;
  UINT64                          TrustCacheAddress;
  UINT64                          KernelPrefix;
  UINT64                          KernelAddress;
  UINT64                          VirtualBase;
  DARWIN_BOOT_ARGS                *Args;
  DARWIN_BOOT_ARGS32              *Args32;
  UINTN                           SeedOffset;
  UINTN                           OutsideOffset;
  UINTN                           OutsideBytes;
  UINT64                          Seed[8];
  EFI_MEMORY_DESCRIPTOR           *Map;
  UINTN                           Capacity;
  UINTN                           MapSize;
  UINTN                           Key;
  UINTN                           Stride;
  UINT32                          Version;
  UINTN                           Attempt;
  CHAR8                           CommandLine[FixedPcdGet32 (PcdXnuCommandLineSize)];
  UINT64                          Video[6];

  (VOID)This;
  if ((Request == NULL) || (Request->Volume == NULL) || (Request->KernelPath == NULL)) { return EFI_INVALID_PARAMETER; }
  if ((Request->KextCount != 0) || (Request->DeviceTree != NULL) || (Request->DeviceTreeSize != 0) ||
      ((DarwinCurrentEl () != 4) && (DarwinCurrentEl () != 8))) {
    Print (L"DARWINBOOT: unsupported inputs or exception level; firmware retained\n");
    return EFI_UNSUPPORTED;
  }
  if (!RamdiskLayout (Request->Ramdisk, Request->RamdiskSize, &RamdiskExpandedSize, &RamdiskPacked)) {
    Print (L"DARWINBOOT: ramdisk layout rejected, %lu bytes\n", (UINT64)Request->RamdiskSize);
    return EFI_BAD_BUFFER_SIZE;
  }
  ZeroMem (CommandLine, sizeof (CommandLine));
  if (Request->BootArgs != NULL) {
    for (Index = 0; Request->BootArgs[Index] != 0; Index++) {
      if ((Index >= sizeof (CommandLine) - 1) || (Request->BootArgs[Index] > 0x7f)) {
        Print (L"DARWINBOOT: boot args rejected at %lu\n", (UINT64)Index);
        return EFI_BAD_BUFFER_SIZE;
      }
      CommandLine[Index] = (CHAR8)Request->BootArgs[Index];
    }
  }
  Root = NULL;
  Image = NULL;
  Tree = NULL;
  TrustCaches = NULL;
  TreePath = NULL;
  Map = NULL;
  Address = 0;
  Capacity = 0;
  Status = gBS->HandleProtocol (Request->Volume, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Fs);
  if (EFI_ERROR (Status)) { return Status; }
  Status = Fs->OpenVolume (Fs, &Root);
  if (EFI_ERROR (Status)) { return Status; }
  Status = ReadFile (Root, Request->KernelPath, SIZE_128MB, &Image, &ImageSize);
  if (EFI_ERROR (Status)) { goto Fail; }
  Status = ParseKernel (Image, ImageSize, &Kernel);
  if (EFI_ERROR (Status)) { goto Fail; }
  if (Kernel.Arm32 && ((DarwinCurrentEl () != 8) || (((READ_SYSREG (id_aa64pfr0_el1) >> 4) & 15) != 2))) {
    Print (L"DARWINBOOT: ARM32 requires EL2 and AArch32 EL1 support; firmware retained\n");
    Status = EFI_UNSUPPORTED;
    goto Fail;
  }
  if (Kernel.Arm32 && Request->Ramdisk != NULL) { Status = EFI_UNSUPPORTED; goto Fail; }
  if (Kernel.Arm32 && AsciiStrLen (CommandLine) >= sizeof (Args32->CommandLine)) { Status = EFI_BAD_BUFFER_SIZE; goto Fail; }
  Length = StrSize (Request->KernelPath);
  TreePath = AllocatePool (Length + 11 * sizeof (CHAR16));
  if (TreePath == NULL) { Status = EFI_OUT_OF_RESOURCES; goto Fail; }
  CopyMem (TreePath, Request->KernelPath, Length);
  StrCatS (TreePath, Length / sizeof (CHAR16) + 11, L".adt");
  Status = ReadFile (Root, TreePath, SIZE_16MB, &Tree, &TreeSize);
  if (EFI_ERROR (Status)) { goto Fail; }
  Offset = 0;
  RamdiskOffset = MAX_UINTN;
  TrustCacheOffset = MAX_UINTN;
  WindowSize = 0;
  KeepPacked = FALSE;
  SeedOffset = MAX_UINTN;
  OutsideOffset = MAX_UINTN;
  OutsideBytes = 0;
  if (!AdtNode (Tree, TreeSize, &Offset, 0, 3, &RamdiskOffset, &TrustCacheOffset, &WindowSize, &KeepPacked, Kernel.Arm32, &SeedOffset, &OutsideOffset, &OutsideBytes) || (Offset != TreeSize)) { Status = EFI_COMPROMISED_DATA; goto Fail; }
  if (Kernel.Arm32 && (SeedOffset == MAX_UINTN)) { Status = EFI_COMPROMISED_DATA; goto Fail; }
  if (SeedOffset != MAX_UINTN) {
    for (Index = 0; Index < ARRAY_SIZE (Seed); Index++) {
      if (!GetRandomNumber64 (&Seed[Index])) { Status = EFI_DEVICE_ERROR; goto Fail; }
    }
    CopyMem ((UINT8 *)Tree + SeedOffset, Seed, sizeof (Seed));
    ZeroMem (Seed, sizeof (Seed));
  }
  if (KeepPacked && (!RamdiskPacked || (RamdiskOffset == MAX_UINTN))) { Status = EFI_COMPROMISED_DATA; goto Fail; }
  if (WindowSize == 0) { WindowSize = FixedPcdGet64 (PcdXnuDefaultWindowSize); }
  if ((WindowSize < BLOCK_SIZE) || !DarwinWindowSizeValid (WindowSize, FixedPcdGet64 (PcdXnuMinWindowSize), FixedPcdGet64 (PcdXnuMaxWindowSize))) { Status = EFI_INVALID_PARAMETER; goto Fail; }
  if (Kernel.Base > MAX_UINT64 - WindowSize) { Status = EFI_LOAD_ERROR; goto Fail; }
  if (Request->Ramdisk != NULL) {
    if ((RamdiskOffset == MAX_UINTN) || (U64 ((UINT8 *)Tree + RamdiskOffset) != 0) ||
        (U64 ((UINT8 *)Tree + RamdiskOffset + 8) != 0)) { Status = EFI_COMPROMISED_DATA; goto Fail; }
  } else if (RamdiskOffset != MAX_UINTN) {
    Status = EFI_UNSUPPORTED;
    goto Fail;
  }
  CopyMem (TreePath, Request->KernelPath, Length);
  StrCatS (TreePath, Length / sizeof (CHAR16) + 11, L".trustcache");
  Status = ReadFile (Root, TreePath, SIZE_16MB, &TrustCaches, &TrustCacheSize);
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) { goto Fail; }
  if (TrustCaches != NULL) {
    if ((TrustCacheOffset == MAX_UINTN) || (Kernel.Arm32 ? (U32 ((UINT8 *)Tree + TrustCacheOffset) != 0 || U32 ((UINT8 *)Tree + TrustCacheOffset + 4) != 0) :
         (U64 ((UINT8 *)Tree + TrustCacheOffset) != 0 || U64 ((UINT8 *)Tree + TrustCacheOffset + 8) != 0)) || !ValidTrustCaches (TrustCaches, TrustCacheSize)) {
      Status = EFI_COMPROMISED_DATA;
      goto Fail;
    }
  } else if (TrustCacheOffset != MAX_UINTN) {
    Status = EFI_COMPROMISED_DATA;
    goto Fail;
  }
  KernelPrefix = Kernel.Arm32 ? (Kernel.Base & (BLOCK_SIZE - 1)) :
                 ((TrustCaches == NULL) ? 0 : ALIGN_VALUE (TrustCacheSize, BLOCK_SIZE));
  if ((Kernel.Base < KernelPrefix) || (KernelPrefix > WindowSize - BLOCK_SIZE) ||
      (Kernel.End - Kernel.Base > WindowSize - BLOCK_SIZE - KernelPrefix)) { Status = EFI_BAD_BUFFER_SIZE; goto Fail; }
  VirtualBase = Kernel.Base - KernelPrefix;
  Status = ReadBootVideo (Video);
  if (EFI_ERROR (Status)) { goto Fail; }
  Status = AllocateWindow (&Kernel, WindowSize, &Address);
  if (EFI_ERROR (Status)) { goto Fail; }
  if ((Address > MAX_UINT64 - WindowSize) || (VirtualBase > MAX_UINT64 - WindowSize)) { Status = EFI_BAD_BUFFER_SIZE; goto Fail; }
  if (Kernel.Arm32 && (VirtualBase > MAX_UINT32 || WindowSize > MAX_UINT32 - VirtualBase ||
      Video[0] > MAX_UINT32 || Video[2] * Video[4] > MAX_UINT32 - Video[0])) { Status = EFI_BAD_BUFFER_SIZE; goto Fail; }
  KernelAddress = Address + KernelPrefix;
  ArgsAddress = ALIGN_VALUE (KernelPrefix + Kernel.End - Kernel.Base, SIZE_16KB);
  if ((ArgsAddress > WindowSize - EFI_PAGE_SIZE) || (TreeSize > WindowSize - ArgsAddress - EFI_PAGE_SIZE)) { Status = EFI_BAD_BUFFER_SIZE; goto Fail; }
  TreeAddress = ArgsAddress + EFI_PAGE_SIZE;
  TrustCacheAddress = ALIGN_VALUE (TreeAddress + TreeSize, 16);
  RamdiskAddress = ALIGN_VALUE (Kernel.Arm32 ? TrustCacheAddress + TrustCacheSize : TreeAddress + TreeSize, EFI_PAGE_SIZE);
  RamdiskBytes = ALIGN_VALUE (KeepPacked ? Request->RamdiskSize : RamdiskExpandedSize, EFI_PAGE_SIZE);
  if ((RamdiskAddress > WindowSize) || (RamdiskBytes > WindowSize - RamdiskAddress)) { Status = EFI_BAD_BUFFER_SIZE; goto Fail; }
  Used = ALIGN_VALUE (RamdiskAddress + RamdiskBytes, SIZE_16KB);
  if (Used > WindowSize - BLOCK_SIZE) {
    Print (L"DARWINBOOT: used %lu window %lu kernel %lu ramdisk %lu\n", (UINT64)Used, (UINT64)WindowSize, (UINT64)(Kernel.End - Kernel.Base), (UINT64)RamdiskBytes);
    Status = EFI_BAD_BUFFER_SIZE;
    goto Fail;
  }
  ArgsAddress += Address;
  TreeAddress += Address;
  RamdiskAddress += Address;
  TrustCacheAddress = Kernel.Arm32 ? Address + TrustCacheAddress : Address;
  ZeroMem ((VOID *)(UINTN)Address, (UINTN)Used);
  for (Index = 0; Index < Kernel.Count; Index++) {
    CopyMem ((VOID *)(UINTN)(KernelAddress + Kernel.Segments[Index].Address - Kernel.Base),
             (UINT8 *)Image + Kernel.Segments[Index].FileOffset, (UINTN)Kernel.Segments[Index].FileSize);
  }
  if (Kernel.Arm32) {
    Args32 = (DARWIN_BOOT_ARGS32 *)(UINTN)ArgsAddress;
    Args32->Revision = 2;
    Args32->Version = 2;
    Args32->VirtualBase = (UINT32)VirtualBase;
    Args32->PhysicalBase = (UINT32)Address;
    Args32->MemorySize = (UINT32)WindowSize;
    Args32->MemorySizeActual = (UINT32)WindowSize;
    Args32->TopOfKernelData = (UINT32)(Address + Used);
    for (Index = 0; Index < ARRAY_SIZE (Video); Index++) { Args32->Video[Index] = (UINT32)Video[Index]; }
    Args32->DeviceTree = (UINT32)(VirtualBase + TreeAddress - Address);
    Args32->DeviceTreeLength = (UINT32)TreeSize;
    AsciiStrCpyS (Args32->CommandLine, sizeof (Args32->CommandLine), CommandLine);
  } else {
  Args = (DARWIN_BOOT_ARGS *)(UINTN)ArgsAddress;
  Args->Revision = 2;
  Args->Version = 2;
  Args->VirtualBase = VirtualBase;
  Args->PhysicalBase = Address;
  Args->MemorySize = WindowSize;
  Args->MemorySizeActual = WindowSize;
  Args->TopOfKernelData = Address + Used;
  CopyMem (Args->Video, Video, sizeof (Video));
  Args->DeviceTree = VirtualBase + TreeAddress - Address;
  Args->DeviceTreeLength = (UINT32)TreeSize;
  CopyMem (Args->CommandLine, CommandLine, sizeof (CommandLine));
  }
  CopyMem ((VOID *)(UINTN)TreeAddress, Tree, TreeSize);
  if (Request->Ramdisk != NULL) {
    if (KeepPacked) { CopyMem ((VOID *)(UINTN)RamdiskAddress, Request->Ramdisk, Request->RamdiskSize); }
    else { CopyRamdisk ((VOID *)(UINTN)RamdiskAddress, Request->Ramdisk, Request->RamdiskSize, RamdiskPacked); }
    WriteUnaligned64 ((UINT64 *)(UINTN)(TreeAddress + RamdiskOffset), RamdiskAddress);
    WriteUnaligned64 ((UINT64 *)(UINTN)(TreeAddress + RamdiskOffset + 8), KeepPacked ? Request->RamdiskSize : RamdiskBytes);
  }
  if (TrustCaches != NULL) {
    CopyMem ((VOID *)(UINTN)TrustCacheAddress, TrustCaches, TrustCacheSize);
    if (Kernel.Arm32) {
      WriteUnaligned32 ((UINT32 *)(UINTN)(TreeAddress + TrustCacheOffset), (UINT32)TrustCacheAddress);
      WriteUnaligned32 ((UINT32 *)(UINTN)(TreeAddress + TrustCacheOffset + 4), (UINT32)TrustCacheSize);
    } else {
      WriteUnaligned64 ((UINT64 *)(UINTN)(TreeAddress + TrustCacheOffset), TrustCacheAddress);
      WriteUnaligned64 ((UINT64 *)(UINTN)(TreeAddress + TrustCacheOffset + 8), TrustCacheSize);
    }
    FreePool (TrustCaches); TrustCaches = NULL;
  }
  FreePool (Image); Image = NULL;
  FreePool (Tree); Tree = NULL;
  FreePool (TreePath); TreePath = NULL;
  Root->Close (Root); Root = NULL;
  Status = GetMap (&Map, &Capacity, &MapSize, &Key, &Stride);
  if (EFI_ERROR (Status)) { goto Fail; }
  Print (L"DARWINBOOT: %a va=0x%lx pa=0x%lx args=0x%lx entry=0x%lx owned=0x%lx\n",
         Kernel.Arm32 ? "ARM32" : "ARM64", Kernel.Base, KernelAddress, ArgsAddress, KernelAddress + Kernel.Entry - Kernel.Base, (UINT64)WindowSize);
  Print (L"DARWINBOOT: video base=0x%lx display=%lu row=%lu width=%lu height=%lu depth=%lu\n",
         Video[0], Video[1], Video[2], Video[3], Video[4], Video[5]);
  Print (L"DARWINBOOT: command line: %a\n", CommandLine);
  Print (L"DARWINBOOT: inputs validated; exiting boot services\n");

  //
  // The same facts in large type over the whole screen. The firmware's log
  // draws far larger than the text console, and the form feed clears the
  // screen first. This is what stays visible if the kernel never draws.
  //
  DEBUG ((DEBUG_ERROR, "\fDARWINBOOT: hand-off to the kernel\n\n"));
  DEBUG ((DEBUG_ERROR, "kernel   va 0x%lx  pa 0x%lx\n", Kernel.Base, KernelAddress));
  DEBUG ((DEBUG_ERROR, "entry    pa 0x%lx\n", KernelAddress + Kernel.Entry - Kernel.Base));
  DEBUG ((DEBUG_ERROR, "args     pa 0x%lx  window 0x%lx bytes\n", ArgsAddress, (UINT64)WindowSize));
  DEBUG ((DEBUG_ERROR, "video    base 0x%lx  %lux%lu  row %lu bytes  %lu bpp\n", Video[0], Video[3], Video[4], Video[2], Video[5]));
  DEBUG ((DEBUG_ERROR, "entry words %08x %08x %08x %08x\n",
          ReadUnaligned32 ((CONST UINT32 *)(UINTN)(KernelAddress + Kernel.Entry - Kernel.Base)),
          ReadUnaligned32 ((CONST UINT32 *)(UINTN)(KernelAddress + Kernel.Entry - Kernel.Base + 4)),
          ReadUnaligned32 ((CONST UINT32 *)(UINTN)(KernelAddress + Kernel.Entry - Kernel.Base + 8)),
          ReadUnaligned32 ((CONST UINT32 *)(UINTN)(KernelAddress + Kernel.Entry - Kernel.Base + 12))));
  DEBUG ((DEBUG_ERROR, "CurrentEL %lu  SCTLR_EL1 0x%lx  TCR_EL1 0x%lx\n", DarwinCurrentEl () >> 2, READ_SYSREG (sctlr_el1), READ_SYSREG (tcr_el1)));
  DEBUG ((DEBUG_ERROR, "ID_AA64MMFR0_EL1 0x%lx  (PARange %lu, 4 KiB granule %a)\n", READ_SYSREG (id_aa64mmfr0_el1),
          READ_SYSREG (id_aa64mmfr0_el1) & 0xF, (((READ_SYSREG (id_aa64mmfr0_el1) >> 28) & 0xF) == 0xF) ? "NOT supported" : "supported"));
  DEBUG ((DEBUG_ERROR, "ID_AA64PFR0_EL1  0x%lx\n", READ_SYSREG (id_aa64pfr0_el1)));
  DEBUG ((DEBUG_ERROR, "\nExiting boot services.\n"));
  for (Attempt = 0; Attempt < 3; Attempt++) {
    MapSize = Capacity;
    Status = gBS->GetMemoryMap (&MapSize, Map, &Key, &Stride, &Version);
    if (EFI_ERROR (Status)) {
      if (Attempt == 0) { goto Fail; }
      CpuDeadLoop ();
    }
    if (OutsideOffset != MAX_UINTN) {
      Status = DescribeOutsideMemory (Map, MapSize, Stride,
                 (UINT8 *)(UINTN)(TreeAddress + OutsideOffset), OutsideBytes);
      if (Status == EFI_BUFFER_TOO_SMALL) {
        // A refreshed map may have fragmented. Fall back to the allocated window.
        ZeroMem ((VOID *)(UINTN)(TreeAddress + OutsideOffset), OutsideBytes);
        Status = EFI_SUCCESS;
      }
      if (EFI_ERROR (Status)) {
        if (Attempt == 0) { goto Fail; }
        CpuDeadLoop ();
      }
    }
    Status = gBS->ExitBootServices (mImage, Key);
    if (!EFI_ERROR (Status)) {
      DarwinEnter (ArgsAddress, KernelAddress + Kernel.Entry - Kernel.Base, Address, WindowSize, Video[0], Video[2] * Video[4], Kernel.Arm32);
      CpuDeadLoop ();
    }
    if (Status != EFI_INVALID_PARAMETER) { CpuDeadLoop (); }
  }
  CpuDeadLoop ();
  return EFI_ABORTED;
Fail:
  if (Root != NULL) { Root->Close (Root); }
  if (Image != NULL) { FreePool (Image); }
  if (Tree != NULL) { FreePool (Tree); }
  if (TrustCaches != NULL) { FreePool (TrustCaches); }
  if (TreePath != NULL) { FreePool (TreePath); }
  if (Map != NULL) { FreePool (Map); }
  if (Address != 0) { gBS->FreePages (Address, EFI_SIZE_TO_PAGES (WindowSize)); }
  Print (L"DARWINBOOT: boot refused: %r; firmware retained\n", Status);
  return Status;
}

STATIC UNIXNU_LOADER_PROTOCOL mLoader = { UNIXNU_LOADER_REVISION, Boot };

STATIC
EFI_STATUS
EFIAPI
Unload (EFI_HANDLE ImageHandle)
{
  return gBS->UninstallMultipleProtocolInterfaces (ImageHandle, &gUniXNULoaderProtocolGuid, &mLoader, NULL);
}

EFI_STATUS
EFIAPI
DarwinBootEntry (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_LOADED_IMAGE_PROTOCOL *Loaded;
  EFI_STATUS               Status;

  (VOID)SystemTable;
  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Loaded);
  if (EFI_ERROR (Status)) { return Status; }
  mImage = ImageHandle;
  Status = gBS->InstallMultipleProtocolInterfaces (&ImageHandle, &gUniXNULoaderProtocolGuid, &mLoader, NULL);
  if (!EFI_ERROR (Status)) {
    Loaded->Unload = Unload;
    Print (L"DARWINBOOT: loader v2 installed EL%u\n", (UINT32)(DarwinCurrentEl () >> 2));
  }
  return Status;
}
