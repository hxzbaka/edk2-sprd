Device (DPU0)
{
    Name (_HID, "SPRD0014")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x20300000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x3B }
    })
}

Device (GSP0)
{
    Name (_HID, "SPRD0015")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x20300000, 0x00002000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x3A }
    })
}

Device (DIOM)
{
    Name (_HID, "SPRD0016")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x20300000, 0x00000800)
        Memory32Fixed (ReadWrite, 0x20300800, 0x00000080)
    })
}

Device (DSI0)
{
    Name (_HID, "SPRD0017")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x20400000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x3E, 0x3F }
    })
}

Device (DPHY)
{
    Name (_HID, "SPRD0018")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x20400000, 0x00001000)
    })
}

Device (FBR0)
{
    Name (_HID, "SPRD0019")
    Name (_UID, Zero)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x9EB0C000, 0x0012D000)
    })
}
