#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {

/* DDR before FB */
{"RAM_LOW",
 0x80000000,
 0x1E000000,
 AddMem, SYS_MEM, SYS_MEM_CAP,
 Conv, WRITE_BACK_XN},

/* FB */
{"FrameBuffer",
 0x9E000000,
 0x01000000,
 AddDev, MMAP_IO, UNCACHEABLE,
 MmIO, NS_DEVICE},

/* DDR between FB and FD */
{"RAM_MID",
 0xA0000000,
 0x2E000000,
 AddMem, SYS_MEM, SYS_MEM_CAP,
 Conv, WRITE_BACK_XN},

/* FD */
{"UEFI_FD",
 0xCE000000,
 0x02000000,
 AddMem, SYS_MEM, SYS_MEM_CAP,
 BsData, WRITE_BACK},

/* DDR above FD */
{"RAM_HIGH",
 0xD0000000,
 0xB0000000,
 AddMem, SYS_MEM, SYS_MEM_CAP,
 Conv, WRITE_BACK_XN},



{"SEC Heap",       0x9FF00000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
{"CPU Vectors",    0x9FF40000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
{"MMU PageTables", 0x9FF41000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
{"UEFI Stack",     0x9FF80000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},

{"Boot Scratch",   0x9FFC0000, 0x00020000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
{"Log Buffer",     0x9FFE0000, 0x00010000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
{"Info Block",     0x9FFF0000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},

/* =========================
 * 4. SPRD MMIO ONLY
 * ========================= */

/* GIC */
{"GIC Distributor",   0x12000000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
{"GIC Redistributor", 0x12040000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

/* UART */
{"UART1", 0x70100000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

/* SDIO */
{"EMMC", 0x71400000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

/* AP / AON */
{"AP_AHB",  0x71000000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
{"AON_APB", 0x327D0000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

/* =========================
 * TERMINATOR
 * ========================= */

{"Terminator", 0, 0, 0, 0, 0, 0, 0}
};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
  return gDeviceMemoryDescriptorEx;
}