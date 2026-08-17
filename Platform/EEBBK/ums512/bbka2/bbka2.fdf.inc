// per-device BSP DXEs

// ACPI Tables
FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/DSDT/DSDT.aml
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/CSRT.aml
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/DBG2.aml
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/FACS.aml
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/FADT.aml
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/GTDT.aml
  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/MADT.aml
//  SECTION RAW = Silicon/Spreadtrum/ums512/AcpiTables/BuiltIn/SPCR.aml
  SECTION UI = "AcpiTables"
}

// Mainline device tree blob