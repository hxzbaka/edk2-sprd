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
        Memory32Fixed (ReadWrite, 0x71400000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2A }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "bus-width", 0x08 },
            Package () { "non-removable", One },
            Package () { "cap-mmc-hw-reset", One },
            Package () { "mmc-hs400-1_8v", One },
            Package () { "mmc-hs400-enhanced-strobe", One },
            Package () { "mmc-hs200-1_8v", One },
            Package () { "mmc-ddr-1_8v", One }
        }
    })
}
