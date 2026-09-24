// UMS512 CPU topology and PSCI low-power idle states.
// DTS: PSCI 0.2 over SMC, 6x Cortex-A55 and 2x Cortex-A75.

Device (SYSM)
{
    Name (_HID, "ACPI0010")
    Name (_UID, Zero)

    Device (CL00)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, Zero)
        Device (CPU0) { Name (_HID, "ACPI0007") Name (_UID, Zero) Method (_LPI, 0, NotSerialized) { Return (A55L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
        Device (CPU1) { Name (_HID, "ACPI0007") Name (_UID, One)  Method (_LPI, 0, NotSerialized) { Return (A55L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
        Device (CPU2) { Name (_HID, "ACPI0007") Name (_UID, 2)    Method (_LPI, 0, NotSerialized) { Return (A55L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
        Device (CPU3) { Name (_HID, "ACPI0007") Name (_UID, 3)    Method (_LPI, 0, NotSerialized) { Return (A55L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
        Device (CPU4) { Name (_HID, "ACPI0007") Name (_UID, 4)    Method (_LPI, 0, NotSerialized) { Return (A55L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
        Device (CPU5) { Name (_HID, "ACPI0007") Name (_UID, 5)    Method (_LPI, 0, NotSerialized) { Return (A55L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
    }

    Device (CL01)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, One)
        Device (CPU6) { Name (_HID, "ACPI0007") Name (_UID, 6) Method (_LPI, 0, NotSerialized) { Return (A75L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
        Device (CPU7) { Name (_HID, "ACPI0007") Name (_UID, 7) Method (_LPI, 0, NotSerialized) { Return (A75L) } Method (_STA, 0, NotSerialized) { Return (0x0F) } }
    }

    // The FFixedHW address is the PSCI power-state parameter. 0xffffffff
    // selects WFI; 0x00010000 is the only power-down state in t610w.dts.
    Name (A55L, Package ()
    {
        Zero, Zero, 2,
        Package ()
        {
            Zero, Zero, Zero, Zero, Zero, Zero,
            ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000000FFFFFFFF, 3) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            "UMS512.A55.WFI"
        },
        Package ()
        {
            2500, 1500, Zero, One, Zero, Zero,
            ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000000010000, 3) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            "UMS512.A55.PSCI-CORE-PD"
        }
    })

    Name (A75L, Package ()
    {
        Zero, Zero, 2,
        Package ()
        {
            Zero, Zero, Zero, Zero, Zero, Zero,
            ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000000FFFFFFFF, 3) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            "UMS512.A75.WFI"
        },
        Package ()
        {
            10000, 8000, Zero, One, Zero, Zero,
            ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000000010000, 3) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
            "UMS512.A75.PSCI-CORE-PD"
        }
    })

}
