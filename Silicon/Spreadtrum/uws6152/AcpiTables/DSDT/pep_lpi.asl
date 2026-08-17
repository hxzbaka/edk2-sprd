// UWS6152 CPU topology + LPI: 3x Cortex-A55 (cluster0) + 1x Cortex-A75 (cluster1)

Device (SYSM)
{
    Name (_HID, "ACPI0010")
    Name (_UID, 0x00)

    Device (CL00)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, 0x10)

        Name (_LPI, Package ()
        {
            0,
            0x01000000,
            2,
            Package ()
            {
                300, 130, Zero, Zero, Zero, Zero, 0x20,
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                "L2_A55.D2d"
            },
            Package ()
            {
                6400, 900, One, Zero, Zero, 0x03, 0x40,
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                "L2_A55.D4"
            }
        })

        Device (CPU0)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, Zero)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000000FFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C1"
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000000000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C2d"
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000040000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C3"
                }
            })
        }

        Device (CPU1)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, One)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000000FFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C1"
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000000000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C2d"
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000040000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C3"
                }
            })
        }

        Device (CPU2)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x02)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000000FFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C1"
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000000000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C2d"
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000040000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A55.C3"
                }
            })
        }
    }

    Device (CL01)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, 0x20)

        Name (_LPI, Package ()
        {
            0,
            0x01000000,
            2,
            Package ()
            {
                900, 200, Zero, Zero, Zero, Zero, 0x20,
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                "L2_A75.D2d"
            },
            Package ()
            {
                6000, 1200, One, Zero, Zero, 0x03, 0x40,
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                "L2_A75.D4"
            }
        })

        Device (CPU3)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x03)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000000FFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A75.C1"
                },
                Package ()
                {
                    600, 80, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000000000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A75.C2d"
                },
                Package ()
                {
                    7000, 90, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x0000000040000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UWS6152.A75.C3"
                }
            })
        }
    }
}
