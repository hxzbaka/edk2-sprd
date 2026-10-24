#include <Base.h>
#include <Guid/SmBios.h>
#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>
#include <Library/SOCSmbiosInfoLib.h>

/***********************************************************************
        SMBIOS data definition  TYPE4  Processor Information
************************************************************************/
SMBIOS_TABLE_TYPE4 mProcessorInfoType4_a75 = {
    {EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof(SMBIOS_TABLE_TYPE4), 0},
    1,                // Socket String
    CentralProcessor,
    ProcessorFamilyIndicatorFamily2,
    2,                // ProcessorManufacture String
    {                 // ProcessorId
     {0x00, 0x00, 0x00, 0x00},
     {0x00, 0x00, 0x00, 0x00}},
    3,                // ProcessorVersion String
    { 0, 0, 0, 0, 0, 1 }, // Voltage
    0,                // ExternalClock
    1800,             // MaxSpeed
    1800,             // CurrentSpeed
    0x41,             // Status
    ProcessorUpgradeOther,
    0,                // L1CacheHandle (updated at runtime)
    0,                // L2CacheHandle (updated at runtime)
    0xFFFF,           // L3CacheHandle
    0,                // SerialNumber
    0,                // AssetTag
    0,                // PartNumber
    2,                // CoreCount
    2,                // EnabledCoreCount
    0,                // ThreadCount
    0xEC,             // ProcessorCharacteristics
    0xC4,             // ProcessorFamily2 (Cortex-A75)
    0, 0, 0
};

SMBIOS_TABLE_TYPE4 mProcessorInfoType4_a55 = {
    {EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof(SMBIOS_TABLE_TYPE4), 0},
    1,                // Socket String
    CentralProcessor,
    ProcessorFamilyIndicatorFamily2,
    2,
    { {0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x00, 0x00} },
    3,
    { 0, 0, 0, 0, 0, 1 },
    0,
    1800,
    1800,
    0x41,
    ProcessorUpgradeOther,
    0, 0, 0xFFFF, 0, 0, 0,
    6,                // CoreCount
    6,                // EnabledCoreCount
    0,
    0xEC,
    0xD0,             // ProcessorFamily2 (Cortex-A55)
    0, 0, 0
};

CHAR8 mCpuName[128] = "UNISOC Tiger T610";
CHAR8 *mProcessorInfoType4Strings[] = {
    "BGA",
    "Spreadtrum",
    "UNISOC Tiger T610",
    NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE7  Cache Information
************************************************************************/

// Cortex-A75 L1 Instruction: 48 KB, 3-way (Other), Unknown mode
SMBIOS_TABLE_TYPE7 mCacheInfoType7_a75_L1I = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    1,
    0x281,
    0x0030, 0x0030,
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    0,
    CacheErrorParity,
    CacheTypeInstruction,
    CacheAssociativityOther
};

// Cortex-A55 L1 Instruction: 32 KB, 2-way
SMBIOS_TABLE_TYPE7 mCacheInfoType7_a55_L1I = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    1,
    0x281,
    0x0020, 0x0020,
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    0,
    CacheErrorParity,
    CacheTypeInstruction,
    CacheAssociativity2Way
};

// Cortex-A75 L1 Data: 32 KB, 2-way, WB
SMBIOS_TABLE_TYPE7 mCacheInfoType7_a75_L1D = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    2,
    0x181,
    0x0020, 0x0020,
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    0,
    CacheErrorSingleBit,
    CacheTypeData,
    CacheAssociativity2Way
};

// Cortex-A55 L1 Data: 32 KB, 4-way, WB
SMBIOS_TABLE_TYPE7 mCacheInfoType7_a55_L1D = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    2,
    0x181,
    0x0020, 0x0020,
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0 },
    0,
    CacheErrorSingleBit,
    CacheTypeData,
    CacheAssociativity4Way
};

// Cortex-A75 L2: 256 KB, 8-way, Unified, WB
SMBIOS_TABLE_TYPE7 mCacheInfoType7_a75_L2 = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    3,
    0x182,
    0x0100, 0x0100,
    { 0, 0, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0 },
    0,
    CacheErrorSingleBit,
    CacheTypeUnified,
    CacheAssociativity8Way
};

// Cortex-A55 L2: 128 KB, 8-way, Unified, WB
SMBIOS_TABLE_TYPE7 mCacheInfoType7_a55_L2 = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    3,
    0x182,
    0x0080, 0x0080,
    { 0, 0, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0 },
    0,
    CacheErrorSingleBit,
    CacheTypeUnified,
    CacheAssociativity8Way
};

CHAR8 *mCacheInfoType7Strings[] = {
    "L1 Instruction",
    "L1 Data",
    "L2",
    NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE17  Memory Device Information
************************************************************************/
SMBIOS_TABLE_TYPE17 mMemDevInfoType17 = {
    {EFI_SMBIOS_TYPE_MEMORY_DEVICE, sizeof(SMBIOS_TABLE_TYPE17), 0},
    0,      // MemoryArrayHandle (filled at runtime)
    0xFFFE,
    64, 64,     // TotalWidth, DataWidth
    0x1000,     // Size: 4096 MB (4 GiB)
    MemoryFormFactorRowOfChips,
    0,
    1,          // DeviceLocator "Builtin"
    2,          // BankLocator "BANK 0"
    MemoryTypeLpddr4,
    {   // TypeDetail
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        1, 0    // Unbuffered
    },
    1866,       // Speed
    2,          // Manufacturer
    0, 0, 0,    // Serial, Asset, Part
    0,          // Attributes
    0,          // ExtendedSize
    0,          // ConfiguredMemoryClockSpeed
    0, 0, 0,    // Min/Max/Configured Voltage
    MemoryTechnologyDram,
    {{  // MemoryOperatingModeCapability (nested struct)
        0,  // Reserved
        0,  // Other
        0,  // Unknown
        1,  // VolatileMemory
        0,  // ByteAccessiblePersistentMemory
        0,  // BlockAccessiblePersistentMemory
        0   // Reserved (10 bits)
    }},
    0, 0, 0, 0, 0,
    0,
    0xFFFFFFFFFFFFFFFFULL, // VolatileSize (filled at runtime)
    0, 0, 0, 0
};

CHAR8 *mMemDevInfoType17Strings[] = {
    "Builtin",
    "BANK 0",
    NULL
};

/***********************************************************************
        RegisterSOCSmbiosInfo
************************************************************************/
VOID
RegisterSOCSmbiosInfo (
    SMBIOS_LOG_SMBIOS_DATA LogSmbiosData,
    EFI_SMBIOS_HANDLE      Type16
    )
{
    EFI_SMBIOS_HANDLE SmbiosHandle;

    // A75 caches
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a75_L1I,
                   mCacheInfoType7Strings, NULL);
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a75_L1D,
                   mCacheInfoType7Strings, &SmbiosHandle);
    mProcessorInfoType4_a75.L1CacheHandle = (UINT16)SmbiosHandle;

    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a75_L2,
                   mCacheInfoType7Strings, &SmbiosHandle);
    mProcessorInfoType4_a75.L2CacheHandle = (UINT16)SmbiosHandle;

    // A55 caches
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a55_L1I,
                   mCacheInfoType7Strings, NULL);
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a55_L1D,
                   mCacheInfoType7Strings, &SmbiosHandle);
    mProcessorInfoType4_a55.L1CacheHandle = (UINT16)SmbiosHandle;

    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a55_L2,
                   mCacheInfoType7Strings, &SmbiosHandle);
    mProcessorInfoType4_a55.L2CacheHandle = (UINT16)SmbiosHandle;

    // Processors
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mProcessorInfoType4_a75,
                   mProcessorInfoType4Strings, NULL);
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mProcessorInfoType4_a55,
                   mProcessorInfoType4Strings, NULL);

    // Memory Device
    mMemDevInfoType17.MemoryArrayHandle = Type16;
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER *)&mMemDevInfoType17,
                   mMemDevInfoType17Strings, NULL);
}