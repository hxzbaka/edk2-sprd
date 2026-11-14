// per-device BSP DXEs

// ACPI Tables
FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/DSDT/DSDT.aml
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/CSRT.aml
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/DBG2.aml
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/FACS.aml
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/FADT.aml
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/GTDT.aml
  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/MADT.aml
# SPCR removed: declares same UART as DBG2 but with PL011 type - Windows HAL
# would init SPRD UART with PL011 register layout and kill kdserial's output.
# WinPE uses graphical console; serial debug comes via DBG2 + kdserial.
#  SECTION RAW = Silicon/Spreadtrum/ums9620_t157/AcpiTables/BuiltIn/SPCR.aml
  SECTION UI = "AcpiTables"
}

// Mainline device tree blob