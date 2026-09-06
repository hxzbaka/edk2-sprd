Device (SYSM)
{
    Name (_HID, "ACPI0010")
    Name (_UID, Zero)

    Device (CL00)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, 0x10)

        Device (CPU0)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, Zero)
            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }
            Name (_LPI, Package ()
            {
                Zero, Zero, One,
                Package ()
                {
                    0x09C4, 0x01F4, One, Zero, Zero, Zero,
                    0x0000000000010000,
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.PSCI"
                }
            })
        }

        Device (CPU1)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, One)
            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }
            Name (_LPI, Package ()
            {
                Zero, Zero, One,
                Package ()
                {
                    0x09C4, 0x01F4, One, Zero, Zero, Zero,
                    0x0000000000010000,
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.PSCI"
                }
            })
        }

        Device (CPU2)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x02)
            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }
            Name (_LPI, Package ()
            {
                Zero, Zero, One,
                Package ()
                {
                    0x09C4, 0x01F4, One, Zero, Zero, Zero,
                    0x0000000000010000,
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.PSCI"
                }
            })
        }
    }

    Device (CL01)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, 0x20)

        Device (CPU3)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x03)
            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }
            Name (_LPI, Package ()
            {
                Zero, Zero, One,
                Package ()
                {
                    0x2710, 0x0FA0, One, Zero, Zero, Zero,
                    0x0000000000010000,
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A75.PSCI"
                }
            })
        }
    }
}
