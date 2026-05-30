#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>
#include <Uefi.h>

// Low RAM configuration (1 GB)
static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx_low[] = {
    /* System RAM */
    {"RAM",          0x80000000, 0x3FFC0000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"UEFI FD",            0x81000000, 0x02000000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"FrameBuffer",        0x9E000000, 0x01000000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED},  // Updated
    {"UEFI Stack",         0xBFC00000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"CPU Vectors",        0xBFC40000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"MMU PageTables",     0xBFC41000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"SEC Heap",           0xBFF44000, 0x0008C000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},

    /* Minimal MMIO for boot */
    {"UART1",              0x70100000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"GIC Distributor",    0x12000000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"GIC Redistributor",  0x12040000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

    {"Terminator", 0, 0, 0, 0, 0, 0, 0}
};

// High RAM configuration (>4 GB)
static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* System RAM */
    {"RAM",          0x80000000, 0x3F000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"UEFI Stack",         0xBFC00000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"CPU Vectors",        0xBFC40000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"MMU PageTables",     0xBFC41000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"UEFI FD",            0x81000000, 0x02000000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
    {"SEC Heap",           0xBFF44000, 0x0008C000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"DXE Heap",           0xC0000000, 0x20000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
    {"FrameBuffer",        0x9E000000, 0x01000000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, UNCACHED_UNBUFFERED},  // Updated

    // High memory above 4GB (if present)
    {"HLOS1",        0x100000000, 0x100000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN},
    {"HLOS2",        0x200000000, 0x100000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN},

    /* Minimal MMIO */
    {"UART1",              0x70100000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"GIC Distributor",    0x12000000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"GIC Redistributor",  0x12040000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

    {"Terminator", 0, 0, 0, 0, 0, 0, 0}
};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
    if (FixedPcdGet64(PcdLowRamSupport)) {
        return gDeviceMemoryDescriptorEx_low;
    } else {
        return gDeviceMemoryDescriptorEx;
    }
}