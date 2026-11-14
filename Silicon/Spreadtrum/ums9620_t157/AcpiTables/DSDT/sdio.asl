Device (SDIO)
{
    Name (_HID, "SPRD0002")
    Name (_UID, One)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x71100000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x33 }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "bus-width", 0x04 },
            Package () { "no-sdio", One },
            Package () { "no-mmc", One }
        }
    })
}

Device (WIFI)
{
    Name (_HID, "SPRD0002")
    Name (_UID, 0x02)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x71200000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x34 }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "bus-width", 0x04 },
            Package () { "no-sd", One },
            Package () { "no-mmc", One }
        }
    })
}
