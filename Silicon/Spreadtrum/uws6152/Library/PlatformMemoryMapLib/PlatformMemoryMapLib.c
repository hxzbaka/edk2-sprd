#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {

  // Register Regions
  { "GICD",              0x12000000,  0x00020000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "GICR",              0x12040000,  0x00100000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_AHB",            0x20100000,  0x00010000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_CLK",            0x20200000,  0x00001000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "DPU",               0x20300000,  0x00002000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "DSI",               0x20400000,  0x00001000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_EIC",            0x32020000,  0x00001000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_GPIO",           0x32070000,  0x00010000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "ADI",               0x32100000,  0x00100000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "USB_PHY_G2",        0x323B0000,  0x00010000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AON_GATE",          0x327D0000,  0x00010000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "PMU_APB",           0x327E0000,  0x00010000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "USB_MUSB",          0x5FFF0000,  0x00002000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "UART1",             0x70100000,  0x00001000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "AP_APB",            0x71000000,  0x00010000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },
  { "SDIO0/EMMC",        0x71400000,  0x00001000,  AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE },

  // DDR Regions & Reserved
  { "ddrbist",           0x80000000,  0x00001000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 0",      0x80001000,  0x053EF000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "warm-boot",         0x853F0000,  0x00010000,  AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  { "System RAM 1",      0x85400000,  0x02000000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "audio",             0x87400000,  0x00400000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "sipc",              0x87800000,  0x00400000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 2",      0x87C00000,  0x01400000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "audiodsp",          0x89000000,  0x00500000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 3",      0x89500000,  0x00100000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "cp-modem",          0x89600000,  0x04600000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 4",      0x8DC00000,  0x06400000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "sml",               0x94000000,  0x00020000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "tos",               0x94020000,  0x003E0000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 5",      0x94400000,  0x03D00000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "iq",                0x98100000,  0x04000000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 6",      0x9C100000,  0x02A0C000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "Writeback",         0x9EB0C000,  0x0012D000,  AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED},
  { "FrameBuffer",       0x9EC39000,  0x00387000,  AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED},
  { "pstore",            0x9EFC0000,  0x00040000,  AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED},
  { "System RAM 7",      0x9F000000,  0x01000000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "UEFI_FD",           0xA0000000,  0x01800000,  AddMem, SYS_MEM, SYS_MEM_CAP, BsCode, WRITE_BACK},
  { "UEFI_RAM",          0xA1800000,  0x1B800000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "UEFI Stack",        0xBD000000,  0x00040000,  AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  { "System RAM 8",      0xBD040000,  0x02F40000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "SEC Heap",          0xBFF80000,  0x00040000,  AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  { "CPU Vectors",       0xBFFC0000,  0x00001000,  AddMem, SYS_MEM, SYS_MEM_CAP, BsCode, WRITE_BACK},
  { "MMU PageTables",    0xBFFC1000,  0x00003000,  AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  { "System RAM 9",      0xBFFC4000,  0x4003C000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN },
  { "System RAM 10",     0x100000000, 0x7FFFF000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  { "Reserved",          0x17FFFF000, 0x00001000,  AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED},

  { "Terminator", 0, 0, 0, 0, 0, 0, 0 }
};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
  return gDeviceMemoryDescriptorEx;
}
