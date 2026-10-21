/** @file
  Platform Memory Map Library for DW99.

  Copyright (c) 2026, hxzbaka. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>
#include <Uefi.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
  // MMIO
  { "GICD",       0x12001000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "GICC",       0x12002000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "GICH",       0x12004000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "GICV",       0x12006000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "SDIO0/EMMC", 0x20600000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "DPU",        0x20800000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_AHB",     0x20E00000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_CLK",     0x21500000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "DSI",        0x21800000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_EIC",     0x40210000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "eFuse",      0x40240000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_GPIO",    0x40280000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "PMU",        0x402B0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AON_APB",    0x402E0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "ADI",        0x40380000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "UART1",      0x70100000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_APB",     0x71300000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },

  // DDR Regions
  { "System RAM 0",      0x80000000,  0x04000000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "wcn-mem",           0x84000000,  0x00300000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "gnss-mem",          0x84300000,  0x0015B000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "System RAM 1",      0x8445B000,  0x033A4000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "mpu0-dump",         0x877FF000,  0x00001000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "sipc-mem",          0x87800000,  0x003D0000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "System RAM 2",      0x87BD0000,  0x01A30000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "cp-modem",          0x89600000,  0x03FC0000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "System RAM 3",      0x8D5C0000,  0x02A40000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "iq-mem",            0x90000000,  0x04000000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "sml-mem",           0x94000000,  0x00020000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "tos-mem",           0x94020000,  0x01FE0000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "System RAM 4",      0x96000000,  0x08000000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "logobuffer",        0x9E000000,  0x00384000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "System RAM 5",      0x9E384000,  0x01B7C000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "SEC Heap",          0x9FF00000,  0x0008C000,  AddMem, SYS_MEM, SYS_MEM_CAP,       BsData, WRITE_BACK_XN       },
  { "CPU Vectors",       0x9FF8C000,  0x00001000,  AddMem, SYS_MEM, SYS_MEM_CAP,       BsData, WRITE_BACK          },
  { "MMU PageTables",    0x9FF8D000,  0x00003000,  AddMem, SYS_MEM, SYS_MEM_CAP,       BsData, WRITE_BACK_XN       },
  { "UEFI Stack",        0x9FF90000,  0x00040000,  AddMem, SYS_MEM, SYS_MEM_CAP,       BsData, WRITE_BACK_XN       },
  { "System RAM 6",      0x9FFD0000,  0x1DF94000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "camera",            0xBDF64000,  0x00500000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "System RAM 7",      0xBE464000,  0x00A8C000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "FrameBuffer",       0xBEEF0000,  0x01000000,  AddMem, MEM_RES, WRITE_COMBINEABLE, Reserv, WRITE_THROUGH_XN    },
  { "System RAM 8",      0xBFEF0000,  0x000D0000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "ramoops",           0xBFFC0000,  0x00040000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },
  { "DXE Heap",          0xC0000000,  0x2DEF0000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "UEFI FD",           0xEDEF0000,  0x02000000,  AddMem, SYS_MEM, SYS_MEM_CAP,       BsData, WRITE_BACK          },
  { "System RAM 9",      0xEFEF0000,  0x10110000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "System RAM 10",     0x100000000, 0x3FFFF000,  AddMem, SYS_MEM, SYS_MEM_CAP,       Conv,   WRITE_BACK_XN       },
  { "Reserved",          0x13FFFF000, 0x00001000,  AddMem, MEM_RES, UNCACHEABLE,       Reserv, UNCACHED_UNBUFFERED },

  { "Terminator",         0, 0, 0, 0, 0, 0, 0}};


ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
    return gDeviceMemoryDescriptorEx;
}
