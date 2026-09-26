// UMS512 CPU topology + LPI: 6x Cortex-A55 (CL00) + 2x Cortex-A75 (CL01)

Device (SYSM)
{
    Name (_HID, "ACPI0010")   // System container
    Name (_UID, 0x00)

    // ---- Cluster 0: 6 x Cortex-A55 ----
    Device (CL00)
    {
        Name (_HID, "ACPI0010")   // Cluster container
        Name (_UID, 0x10)

        // Cluster-level LPI (states that affect the entire cluster)
        Name (_LPI, Package ()
        {
            0,                     // State index
            0x01000000,            // Flags
            2,                     // Number of states
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

        // ---- CPU0 (A55) ----
        Device (CPU0)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C1"     // WFI
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C2d"    // Core power collapse
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C3"     // Core + cluster power collapse
                }
            })
        }


        Device (CPU1)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 1)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C1"     // WFI
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C2d"    // Core power collapse
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C3"     // Core + cluster power collapse
                }
            })
        }

        Device (CPU2)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 2)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C1"     // WFI
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C2d"    // Core power collapse
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C3"     // Core + cluster power collapse
                }
            })
        }

        Device (CPU3)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 3)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C1"     // WFI
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C2d"    // Core power collapse
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C3"     // Core + cluster power collapse
                }
            })
        }

        Device (CPU4)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 4)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C1"     // WFI
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C2d"    // Core power collapse
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C3"     // Core + cluster power collapse
                }
            })
        }

        Device (CPU5)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 5)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C1"     // WFI
                },
                Package ()
                {
                    400, 100, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C2d"    // Core power collapse
                },
                Package ()
                {
                    450, 300, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A55.C3"     // Core + cluster power collapse
                }
            })
        }
    }

    // ---- Cluster 1: 2 x Cortex-A75 ----
    Device (CL01)
    {
        Name (_HID, "ACPI0010")
        Name (_UID, 0x20)

        // Cluster-level LPI for A75 cluster
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

        // ---- CPU6 (A75) ----
        Device (CPU6)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 6)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A75.C1"
                },
                Package ()
                {
                    600, 80, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A75.C2d"
                },
                Package ()
                {
                    7000, 90, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A75.C3"
                }
            })
        }

        // CPU7 (A75)
        Device (CPU7)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 7)

            Method (_STA, 0, NotSerialized) { Return (0x0F) }

            Name (_LPI, Package ()
            {
                Zero, Zero, 3,
                Package ()
                {
                    Zero, Zero, One, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0xFFFFFFFF, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A75.C1"
                },
                Package ()
                {
                    600, 80, Zero, Zero, Zero, Zero,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x00000002, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A75.C2d"
                },
                Package ()
                {
                    7000, 90, One, One, Zero, 0x03,
                    ResourceTemplate () { Register (FFixedHW, 0x20, 0, 0x40000003, 3) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    ResourceTemplate () { Register (SystemMemory, 0, 0, 0, 0) },
                    "UMS512.A75.C3"
                }
            })
        }
    }
}