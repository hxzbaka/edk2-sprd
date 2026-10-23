INF UniXNUPkg/Application/UniXNU/UniXNU.inf
INF UniXNUPkg/Driver/UniXNUDxe/UniXNUDxe.inf
INF DarwinBootPkg/Driver/DarwinBoot.inf
INF Platform/divo/sc9832e/dw99/Drivers/DarwinPanicRecoveryDxe/DarwinPanicRecoveryDxe.inf

// per-device BSP DXEs
INF Platform/divo/sc9832e/dw99/Drivers/SprdDisplayDxe/SprdDisplayDxe.inf
INF Silicon/Spreadtrum/uws6152/Drivers/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

// ACPI Tables
FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
  SECTION RAW = Platform/divo/sc9832e/AcpiTables/dw99/DSDT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/CSRT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/DBG2.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/FACS.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/FADT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/GTDT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/MADT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/SPCR.aml
  SECTION RAW = Platform/divo/sc9832e/AcpiTables/dw99/PPTT.aml
  SECTION UI = "AcpiTables"
}
