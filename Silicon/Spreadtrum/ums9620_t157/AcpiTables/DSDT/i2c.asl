Device (I2C0)
{
    Name (_HID, "SPRD0006")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70300000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2B }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "clock-frequency", 0x61A80 }
        }
    })
}

Device (I2C1)
{
    Name (_HID, "SPRD0006")
    Name (_UID, One)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70400000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2C }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "clock-frequency", 0x61A80 }
        }
    })
}

Device (I2C2)
{
    Name (_HID, "SPRD0006")
    Name (_UID, 0x02)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70500000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2D }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "clock-frequency", 0x61A80 }
        }
    })
}

Device (I2C3)
{
    Name (_HID, "SPRD0006")
    Name (_UID, 0x03)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70600000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2E }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "clock-frequency", 0x61A80 }
        }
    })
}

Device (I2C4)
{
    Name (_HID, "SPRD0006")
    Name (_UID, 0x04)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70700000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2F }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "clock-frequency", 0x61A80 }
        }
    })
}
