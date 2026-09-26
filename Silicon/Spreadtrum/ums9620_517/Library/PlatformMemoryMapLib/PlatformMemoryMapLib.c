#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* Name               Address     Length      HobOption  ResourceAttribute    ArmAttributes
                                                          ResourceType          MemoryType */

    /* Register regions */
    {"GICD",              0x12000000, 0x00020000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"GICR",              0x12040000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"APMA_DMA",          0x20000000, 0x00004000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"AP_CLK_EXT",        0x20200000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"UART1/DEBUG",       0x20210000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"DPU",               0x20300000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"DSI",               0x20400000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    /* UMS9620/Y70 AON domain GPIO/EIC (0x64000000 + offset) */
    {"Y70_AP_GPIO",       0x64170000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_EIC_DEBOUNCE",  0x64200000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_EIC_BANK1",     0x64210000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_EIC_BANK2",     0x64220000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_EIC_BANK3",     0x64230000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_AON_CLK",       0x64920000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_AON_APB",       0x64900000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_PMU_APB",       0x64910000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_ADI",           0x64400000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_AP_AHB_CLK",    0x20010000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_AP_APB_CLK",    0x71300000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_ANLG_PHY_G0L",  0x64300000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_I2C6",          0x222A0000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"Y70_MUSB",          0x64A00000, 0x00002000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
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
    {"Y70_SDIO3_EMMC",    0x22200000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},  // Y70 eMMC controller (sdio3)
    {"Y70_SDIO0",          0x22210000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},  // Y70 SD card controller (sdio0)


/* DDR Regions */
{"ddrbist/sysdumpinfo", 0x80000000, 0x00002000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Low RAM",           0x80002000, 0x056FE000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"Runtime Data",      0x85700000, 0x00080000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
{"Runtime Code",      0x85780000, 0x00080000, AddMem, SYS_MEM, SYS_MEM_CAP, RtCode, WRITE_BACK_XN},
{"Main RAM p1",       0x85800000, 0x02000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"sipc-mem (FW)",     0x87800000, 0x00800000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"cp-modem (FW)",     0x88000000, 0x17000000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Pre-FV RAM",        0x9F000000, 0x00800000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"FV Region",         0x9F800000, 0x00400000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
{"UEFI FD",           0x9FC00000, 0x00300000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
{"SEC Heap",          0x9FF00000, 0x0008C000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
{"CPU Vectors",       0x9FF8C000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
{"MMU PageTables",    0x9FF8D000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
{"UEFI Stack",        0x9FF90000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
{"Free RSRV1",        0x9FFD0000, 0x00012000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"Capsule Header",    0x9FFE2000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED_XN},
{"Free RSRV2",        0x9FFE3000, 0x00014000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"Log Buffer",        0x9FFF7000, 0x00008000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
{"Info Blk",          0x9FFFF000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
{"cp-modem tail (FW)",0xA0000000, 0x05000000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Main RAM p2",       0xA5000000, 0x09600000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"audio/dsp (FW)",    0xAE600000, 0x01A00000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"FrameBuffer",       0xB0000000, 0x01000000, AddMem, MEM_RES, WRITE_COMBINEABLE, Reserv, WRITE_THROUGH_XN},
{"Pool A p1",         0xB1000000, 0x0AE80000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"uboot_log (FW)",    0xBBE80000, 0x00080000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Pool A p2",         0xBBF00000, 0x01100000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"tos/sml (FW)",      0xBD000000, 0x03FC0000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Pool A p3",         0xC0FC0000, 0x02040000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"sh-mem (FW)",       0xC3000000, 0x00100000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Pool A p4 (PHIT)",  0xC3100000, 0x0AF00000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"UEFI FD (High)",    0xCE000000, 0x02000000, NoHob,  MEM_RES, 0,               BsData, WRITE_BACK},
{"Pool B p1",         0xD0000000, 0x20000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"cma_share (FW)",    0xF0000000, 0x03000000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Pool B p2",         0xF3000000, 0x0CA10000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
{"faceid (FW)",       0xFFA10000, 0x00500000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"RAM Hole/hdcp/rampo",0xFFF10000, 0x000F0000, NoHob,  MEM_RES, 0,               Conv,   UNCACHEABLE},
{"Pool C (4GB+)",     0x100000000, 0x3DEA0000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    /* Terminator */
    {"Terminator", 0, 0, 0, 0, 0, 0, 0}
};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
  return gDeviceMemoryDescriptorEx;
}
