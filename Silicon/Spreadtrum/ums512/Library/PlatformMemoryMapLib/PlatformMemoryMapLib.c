#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* Name               Address     Length      HobOption  ResourceAttribute    ArmAttributes
                                                          ResourceType          MemoryType */

    /* Register regions */
    {"GICD",              0x12000000, 0x00020000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"GICR",              0x12040000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"APMA_DMA",          0x20000000, 0x00004000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_AHB",            0x20100000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_CLK_EXT",        0x20200000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"DPU",               0x20300000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"DSI",               0x20400000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_EIC",            0x32020000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_EIC_BANK1",      0x32030000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_GPIO",           0x32070000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AON_I2C",           0x32060000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"ADI",               0x32100000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_EIC_BANK2",      0x32230000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_EFUSE",          0x32240000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_EIC_BANK3",      0x32270000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"WATCHDOG",          0x322F0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AON_CLK",          0x32080000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"ANA_G2",          0x323B0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"ANA_G3/RPLL",     0x323C0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AON_APB",          0x327D0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"PMU_APB",          0x327E0000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"PINCTRL",          0x32450000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AGMA_DMA",          0x33580000, 0x00004000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"USB",               0x5FFF0000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"UART0/WCN",         0x70000000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"UART1/DEBUG",       0x70100000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_I2C0",           0x70300000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_I2C1",           0x70400000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_I2C2",           0x70500000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_I2C3",           0x70600000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_I2C4",           0x70700000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_APB",            0x71000000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"SDIO0/EMMC",        0x71400000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

    /* DDR Regions */
    {"Low RAM",           0x80000000, 0x05700000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"Runtime Data",      0x85700000, 0x00080000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
    {"Runtime Code",      0x85780000, 0x00080000, AddMem, SYS_MEM, SYS_MEM_CAP, RtCode, WRITE_BACK_XN},
    {"Main RAM 0",        0x85800000, 0x01C00000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"Audio reserved (FW)",    0x87400000, 0x00400000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"SIPC reserved (FW)",     0x87800000, 0x00800000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"SensorHub reserved (FW)",0x88000000, 0x01000000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"AudioDSP reserved (FW)", 0x89000000, 0x00600000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"CP reserved (FW)",       0x89600000, 0x04900000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"Main RAM 1",        0x8DF00000, 0x02100000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"IQ reserved (FW)",  0x90000000, 0x04000000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"SML reserved (FW)", 0x94000000, 0x00040000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"TOS reserved (FW)", 0x94040000, 0x05FC0000, NoHob, MEM_RES, 0, Conv, UNCACHEABLE},
    {"Main RAM 2",        0x9A000000, 0x04000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"FrameBuffer",       0x9E000000, 0x01000000, AddMem, MEM_RES, WRITE_COMBINEABLE, Reserv, WRITE_THROUGH_XN},
    {"Pre-FV RAM",        0x9F000000, 0x00800000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"FV Region",         0x9F800000, 0x00400000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"UEFI FD",           0x9FC00000, 0x00300000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"SEC Heap",          0x9FF00000, 0x0008C000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"CPU Vectors",       0x9FF8C000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"MMU PageTables",    0x9FF8D000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"UEFI Stack",        0x9FF90000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"Free RSRV1",        0x9FFD0000, 0x00012000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"Capsule Header",    0x9FFE2000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED_XN},
    {"Free RSRV2",        0x9FFE3000, 0x0001C000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"Info Blk",          0x9FFFF000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    {"DXE Heap",          0xA0000000, 0x2E000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"UEFI FD (High)",    0xCE000000, 0x02000000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"RAM 3G",          0xD0000000, 0x30000000,  AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM 4G",             0x100000000, 0x7DFA0000,  Mem4G,  SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM 6G",             0x100000000, 0xFDA00000,  Mem6G,  SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM 8G",             0x100000000, 0x17DFA0000, Mem8G,  SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM 10G",            0x100000000, 0x1FDA0000,  Mem10G, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"RAM 12G",            0x100000000, 0x27DFA0000, Mem12G, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},

    /* Terminator */
    {"Terminator", 0, 0, 0, 0, 0, 0, 0}
};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
  return gDeviceMemoryDescriptorEx;
}
