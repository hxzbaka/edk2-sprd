// SPDX-License-Identifier: BSD-2-Clause-Patent
#ifndef DARWIN_BOOT_ARGS_H_
#define DARWIN_BOOT_ARGS_H_

#include <Base.h>
#include <Library/PcdLib.h>

// The command line capacity is part of the target kernel's boot_args ABI.
typedef struct {
  UINT16  Revision;
  UINT16  Version;
  UINT32  Padding;
  UINT64  VirtualBase;
  UINT64  PhysicalBase;
  UINT64  MemorySize;
  UINT64  TopOfKernelData;
  UINT64  Video[6];
  UINT32  MachineType;
  UINT32  TreePadding;
  UINT64  DeviceTree;
  UINT32  DeviceTreeLength;
  CHAR8   CommandLine[FixedPcdGet32 (PcdXnuCommandLineSize)];
  UINT32  FlagsPadding;
  UINT64  BootFlags;
  UINT64  MemorySizeActual;
} DARWIN_BOOT_ARGS;

STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, DeviceTree) == 0x60, "DT ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, Video) == 0x28, "video ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, CommandLine) == 0x6c, "command line ABI");
STATIC_ASSERT ((FixedPcdGet32 (PcdXnuCommandLineSize) > 0) && ((FixedPcdGet32 (PcdXnuCommandLineSize) & 7) == 0), "command line ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, MemorySizeActual) == 0x78 + FixedPcdGet32 (PcdXnuCommandLineSize), "memory ABI");
STATIC_ASSERT (sizeof (DARWIN_BOOT_ARGS) == 0x80 + FixedPcdGet32 (PcdXnuCommandLineSize), "boot_args ABI");

typedef struct {
  UINT16 Revision;
  UINT16 Version;
  UINT32 VirtualBase;
  UINT32 PhysicalBase;
  UINT32 MemorySize;
  UINT32 TopOfKernelData;
  UINT32 Video[6];
  UINT32 MachineType;
  UINT32 DeviceTree;
  UINT32 DeviceTreeLength;
  CHAR8 CommandLine[256];
  UINT32 BootFlags;
  UINT32 MemorySizeActual;
} DARWIN_BOOT_ARGS32;

STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS32, Video) == 0x14, "ARM32 video ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS32, DeviceTree) == 0x30, "ARM32 DT ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS32, CommandLine) == 0x38, "ARM32 command line ABI");
STATIC_ASSERT (sizeof (DARWIN_BOOT_ARGS32) == 0x140, "ARM32 boot_args ABI");

#endif
