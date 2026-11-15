#include <Base.h>
#include <Guid/SmBios.h>
#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>
#include <Library/SOCSmbiosInfoLib.h>

SMBIOS_TABLE_TYPE4 mProcessorInfoType4_a53 = {
    {EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof(SMBIOS_TABLE_TYPE4), 0},
    1,                // Socket String
    CentralProcessor, // ProcessorType
    ProcessorFamilyIndicatorFamily2, // ProcessorFamily
    2,                               // ProcessorManufacture String;
    {                                // ProcessorId;
     {0x00, 0x00, 0x00, 0x00},
     {0x00, 0x00, 0x00, 0x00}},
    3, // ProcessorVersion String;
    {
        // Voltage;
        0, // ProcessorVoltageCapability5V
        0, // ProcessorVoltageCapability3_3V
        0, // ProcessorVoltageCapability2_9V
        0, // ProcessorVoltageCapabilityReserved
        0, // ProcessorVoltageReserved
        1 // ProcessorVoltageIndicateLegacy
    },
    0,                     // ExternalClock;
    1820,                  // MaxSpeed;
    0,                     // CurrentSpeed; DVFS-dependent
    0x41,                  // Status;
    ProcessorUpgradeOther, // ProcessorUpgrade
    0,                     // L1CacheHandle;
    0,                     // L2CacheHandle;
    0xFFFF,                // L3CacheHandle;
    0,                     // SerialNumber;
    0,                     // AssetTag;
    0,                     // PartNumber;
    4,                     // CoreCount;
    4,                     // EnabledCoreCount;
    4,                     // ThreadCount;
    0xEC, // ProcessorCharacteristics
    ProcessorFamilyARM, // ARM Processor Family;
    0,                  // CoreCount2;
    0,                  // EnabledCoreCount2;
    0,                  // ThreadCount2;
};

CHAR8 *mProcessorInfoType4Strings[] = {
    "BGA", "Spreadtrum", "Spreadtrum SC9832E", NULL};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_a53_L1I = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    1,     // SocketDesignation String
    0x380, // Cache Configuration
    0x0030, // Maximum Size
    0x0030, // Install Size
    {
        // Supported SRAM Type
        0, // Other
        1, // Unknown
        0, // NonBurst
        0, // Burst
        0, // PiplelineBurst
        0, // Synchronous
        0, // Asynchronous
        0 // Reserved
    },
    {
        // Current SRAM Type
        0, // Other
        1, // Unknown
        0, // NonBurst
        0, // Burst
        0, // PiplelineBurst
        0, // Synchronous
        0, // Asynchronous
        0 // Reserved
    },
    0,                     // Cache Speed unknown
    CacheErrorParity,      // Error Correction
    CacheTypeInstruction,  // System Cache Type
    CacheAssociativity2Way // Associativity
};
CHAR8 *mCacheInfoType7Strings[] = {"L1 Instruction", "L1 Data", "L2", NULL};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_a53_L1D = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    2,     // SocketDesignation String
    0x180, // Cache Configuration
    0x0020, // Maximum Size
    0x0020, // Install Size
    {
        // Supported SRAM Type
        0, // Other
        1, // Unknown
        0, // NonBurst
        0, // Burst
        0, // PiplelineBurst
        0, // Synchronous
        0, // Asynchronous
        0 // Reserved
    },
    {
        // Current SRAM Type
        0, // Other
        1, // Unknown
        0, // NonBurst
        0, // Burst
        0, // PiplelineBurst
        0, // Synchronous
        0, // Asynchronous
        0 // Reserved
    },
    0,                     // Cache Speed unknown
    CacheErrorSingleBit,   // Error Correction
    CacheTypeData,         // System Cache Type
    CacheAssociativity4Way // Associativity
};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_a53_L2 = {
    {EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof(SMBIOS_TABLE_TYPE7), 0},
    3,      // SocketDesignation String
    0x0181, // Cache Configuration
    0x0800, // Maximum Size
    0x0800, // Install Size
    {
        // Supported SRAM Type
        0, // Other
        1, // Unknown
        0, // NonBurst
        0, // Burst
        0, // PiplelineBurst
        0, // Synchronous
        0, // Asynchronous
        0 // Reserved
    },
    {
        // Current SRAM Type
        0, // Other
        0, // Unknown
        1, // NonBurst
        0, // Burst
        0, // PiplelineBurst
        0, // Synchronous
        0, // Asynchronous
        0 // Reserved
    },
    0,                      // Cache Speed unknown
    CacheErrorSingleBit,    // Error Correction Multi
    CacheTypeUnified,       // System Cache Type
    CacheAssociativity16Way // Associativity
};

SMBIOS_TABLE_TYPE17 mMemDevInfoType17 = {
    {EFI_SMBIOS_TYPE_MEMORY_DEVICE, sizeof(SMBIOS_TABLE_TYPE17), 0},
    0, // MemoryArrayHandle
    0xFFFE, // MemoryErrorInformationHandle; (not provided)
    64,     // TotalWidth; (unknown)
    64,     // DataWidth; (unknown)
    3072,  // Size in MB
    MemoryFormFactorRowOfChips, // FormFactor
    0,                          // DeviceSet;
    1,                          // DeviceLocator String
    2,                          // BankLocator String
    MemoryTypeLpddr4, // MemoryType
    {
        // TypeDetail;
        0, // Reserved
        0, // Other
        0, // Unknown
        0, // FastPaged
        0, // StaticColumn
        0, // PseudoStatic
        0, // Rambus
        0, // Synchronous
        0, // Cmos
        0, // Edo
        0, // WindowDram
        0, // CacheDram
        0, // Nonvolatile
        0, // Registered
        1, // Unbuffered
        0, // Reserved1
    },
    1866,                 // Speed; (unknown)
    2,                    // Manufacturer String
    0,                    // SerialNumber String
    0,                    // AssetTag String
    0,                    // PartNumber String
    0,                    // Attributes; (unknown rank)
    0,                    // ExtendedSize; (since Size < 32GB-1)
    0,                    // ConfiguredMemoryClockSpeed; (unknown)
    0,                    // MinimumVoltage; (unknown)
    0,                    // MaximumVoltage; (unknown)
    0,                    // ConfiguredVoltage; (unknown)
    MemoryTechnologyDram, // MemoryTechnology
    {{
        // MemoryOperatingModeCapability
        0, // Reserved
        0, // Other
        0, // Unknown
        1, // VolatileMemory
        0, // ByteAccessiblePersistentMemory
        0, // BlockAccessiblePersistentMemory
        0 // Reserved
    }},
    0,                     // FirmwareVersion
    0,                     // ModuleManufacturerID (unknown)
    0,                     // ModuleProductID (unknown)
    0,                     // MemorySubsystemControllerManufacturerID (unknown)
    0,                     // MemorySubsystemControllerProductID (unknown)
    0,                     // NonVolatileSize
    0xC0000000ULL,         // VolatileSize
    0,                     // CacheSize
    0,                     // LogicalSize
    0,                     // ExtendedSpeed,
    0                      // ExtendedConfiguredMemorySpeed
};
CHAR8 *mMemDevInfoType17Strings[] = {"Builtin", "BANK 0", NULL};

VOID RegisterSOCSmbiosInfo(
	SMBIOS_LOG_SMBIOS_DATA LogSmbiosData,
	EFI_SMBIOS_HANDLE Type16
){
  EFI_SMBIOS_HANDLE SmbiosHandle;
  LogSmbiosData(
      (EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a53_L1I,
      mCacheInfoType7Strings, NULL);

  LogSmbiosData(
      (EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a53_L1D,
      mCacheInfoType7Strings, &SmbiosHandle);
  mProcessorInfoType4_a53.L1CacheHandle = (UINT16)SmbiosHandle;

  LogSmbiosData(
      (EFI_SMBIOS_TABLE_HEADER *)&mCacheInfoType7_a53_L2,
      mCacheInfoType7Strings, &SmbiosHandle);
  mProcessorInfoType4_a53.L2CacheHandle = (UINT16)SmbiosHandle;

  LogSmbiosData(
      (EFI_SMBIOS_TABLE_HEADER *)&mProcessorInfoType4_a53,
      mProcessorInfoType4Strings, NULL);

  mMemDevInfoType17.MemoryArrayHandle    = Type16;
  LogSmbiosData(
      (EFI_SMBIOS_TABLE_HEADER *)&mMemDevInfoType17, mMemDevInfoType17Strings,
      NULL);
}
