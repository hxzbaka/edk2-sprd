/*
 * SC9832E / GM01 eMMC controller.
 *
 * Source: live recovery DTB (/sys/firmware/fdt):
 *   sdio@20600000, compatible = "sprd,sdhc-r10"
 *   interrupts = <GIC_SPI 60 IRQ_TYPE_LEVEL_HIGH>
 * A GIC SPI is translated to ACPI GSI by adding 32: 60 + 32 = 92.
 */
Device (EMMC)
{
    Name (_HID, "SPRD0002")
    Name (_UID, Zero)
    Name (_CCA, One)

    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }

    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x20600000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 92 }
    })

    /* Capabilities from the live sdio_emmc DT node. */
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "bus-width", 8 },
            Package () { "non-removable", One },
            Package () { "cap-mmc-hw-reset", One },
            Package () { "mmc-hs400-1_8v", One },
            Package () { "mmc-hs400-enhanced-strobe", One },
            Package () { "mmc-hs200-1_8v", One },
            Package () { "mmc-ddr-1_8v", One },
            // Controller-specific booleans consumed by sprd-sdhcr10.c and
            // MMC core; SDIO/SD probing must be disabled for soldered eMMC.
            Package () { "sprd,sdio-adma", One },
            Package () { "no-sdio", One },
            Package () { "no-sd", One },
            Package () { "sprd,name", "sdio_emmc" },
            Package () { "voltage-ranges", Package () { 3000, 3000 } },
            // Calibration values from the live SC9832E sdio@20600000 node.
            Package () { "sprd,hs400es-dly", Package () { 0x5A, 0x7F, 0x30, 0x30 } },
            Package () { "sprd,hs400-dly",   Package () { 0x5A, 0x8C, 0x30, 0x30 } },
            Package () { "sprd,hs200-dly",   Package () { 0x7F, 0x8C, 0x8E, 0x8E } },
            Package () { "sprd,ddr52-dly",   Package () { 0x9F, 0xB3, 0x1E, 0x1E } }
        }
    })
}
