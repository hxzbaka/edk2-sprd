/*
 * ACPI namespace for the Unisoc SC9832E (SharkLE) platform.
 *
 * The fixed ACPI tables provide PSCI, four Cortex-A53 CPUs, GIC-400 and the
 * ARM generic timer.  UART1 is described for the custom SPRD kdcom.dll only:
 * its matching DBG2 entry uses the private ARM serial subtype 0x15.
 *
 * Hardware source: arch/arm64/boot/dts/sprd/sharkle.dtsi
 *   uart1: serial@70100000, reg = <0 0x70100000 0 0x100>,
 *   interrupts = <GIC_SPI 3 IRQ_TYPE_LEVEL_HIGH>.
 */
DefinitionBlock ("DSDT.aml", "DSDT", 0x02, "UNISOC", "SC9832E ", 0x00000001)
{
    Scope (\_SB)
    {
        // Spreadtrum SC9836-compatible UART1; custom kdcom DBG2 subtype 0x15.
        Device (URT1)
        {
            Name (_HID, "SPRD0001")
            Name (_CID, "SPRD0001")
            // Live DT alias is serial1; retain the instance ID used by the
            // Unisoc SPRD0001 URT1 template.
            Name (_UID, One)
            Name (_CCA, One)

            Method (_CRS, 0, Serialized)
            {
                Name (RBUF, ResourceTemplate ()
                {
                    // 32-bit registers, TXD/RXD/LSR/STS1 through offset 0x0c.
                    Memory32Fixed (ReadWrite, 0x70100000, 0x00000100)
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
                    {
                        35  // GIC_SPI 3 + 32
                    }
                })
                Return (RBUF)
            }

            Method (_STA, 0, NotSerialized)
            {
                Return (0x0F)
            }
        }

        Include ("emmc.asl")
    }
}
