// per-device BSP DXEs

// ACPI Tables
FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
  # SC9832E/SharkLE native ACPI tables. MCFG is intentionally absent: this
  # platform has no PCIe ECAM. SPCR uses custom interface type 0x15, matching
  # the SC9836 DBG2/kdcom UART transport; it is not a PL011/SBSA declaration.
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/DSDT/DSDT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/CSRT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/DBG2.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/FADT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/GTDT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/MADT.aml
  SECTION RAW = Silicon/Spreadtrum/sc9832e/AcpiTables/BuiltIn/SPCR.aml
  SECTION UI = "AcpiTables"
}

// Mainline device tree blob