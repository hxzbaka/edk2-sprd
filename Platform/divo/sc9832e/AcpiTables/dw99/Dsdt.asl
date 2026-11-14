DefinitionBlock ("DSDT.aml", "DSDT", 0x02, "UNISOC", "DW99    ", 0x00000001)
{
    Scope (\_SB)
    {
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
                    Name (_STA, 0x0F)
                }

                Device (CPU1)
                {
                    Name (_HID, "ACPI0007")
                    Name (_UID, One)
                    Name (_STA, 0x0F)
                }

                Device (CPU2)
                {
                    Name (_HID, "ACPI0007")
                    Name (_UID, 0x02)
                    Name (_STA, 0x0F)
                }

                Device (CPU3)
                {
                    Name (_HID, "ACPI0007")
                    Name (_UID, 0x03)
                    Name (_STA, 0x0F)
                }
            }
        }

        Device (URT1)
        {
            Name (_HID, "SPRD0001")
            Name (_CID, "SPRD0001")
            Name (_UID, One)
            Name (_CCA, One)
            Name (_STA, 0x0F)
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite, 0x70100000, 0x00000100)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 35 }
            })
        }

        Include ("emmc.asl")

        Device (DPU0)
        {
            Name (_HID, "SPRD0014")
            Name (_UID, Zero)
            Name (_CCA, One)
            Name (_STA, 0x0F)
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite, 0x20800000, 0x00001000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 78 }
            })
        }

        Device (DSI0)
        {
            Name (_HID, "SPRD0017")
            Name (_UID, Zero)
            Name (_CCA, One)
            Name (_STA, 0x0F)
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite, 0x21800000, 0x00001000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 80, 81 }
            })
        }

        Device (FBR0)
        {
            Name (_HID, "SPRD0019")
            Name (_UID, Zero)
            Name (_STA, 0x0F)
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite, 0xBEEF0000, 0x000A1000)
            })
        }

        Device (GPIO)
        {
            Name (_HID, "SPRD0011")
            Name (_UID, Zero)
            Name (_CCA, One)
            Name (_STA, 0x0F)
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite, 0x40280000, 0x00001000)
                // The 16 GPIO banks share one AP GPIO interrupt line.
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 67 }
            })
        }

        Device (I2C3)
        {
            Name (_HID, "SPRD0006")
            Name (_UID, 0x03)
            Name (_CCA, One)
            Name (_STA, 0x0F)
            Name (_CRS, ResourceTemplate ()
            {
                Memory32Fixed (ReadWrite, 0x70800000, 0x00001000)
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 46 }
            })
            Name (_DSD, Package ()
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package () { Package () { "clock-frequency", 100000 } }
            })
        }

        Device (TPD0)
        {
            Name (_HID, "HYNT9217")
            Name (_UID, Zero)
            Name (_STA, 0x0F)
            Name (_DEP, Package () { \_SB.GPIO, \_SB.I2C3 })
            Name (_CRS, ResourceTemplate ()
            {
                I2cSerialBusV2 (0x005A, ControllerInitiated, 100000,
                    AddressingMode7Bit, "\\_SB.I2C3", 0x00,
                    ResourceConsumer, , Exclusive)
                GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly,
                    "\\_SB.GPIO", 0, ResourceConsumer, , ) { 145 }
                GpioInt (Edge, ActiveHigh, Exclusive, PullDefault, 0,
                    "\\_SB.GPIO", 0, ResourceConsumer, , ) { 144 }
            })
            Name (_DSD, Package ()
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package ()
                {
                    Package () { "compatible", "hynitron,hyn_ts" },
                    Package () { "touchscreen-size-x", 368 },
                    Package () { "touchscreen-size-y", 448 }
                }
            })
        }
    }
}
