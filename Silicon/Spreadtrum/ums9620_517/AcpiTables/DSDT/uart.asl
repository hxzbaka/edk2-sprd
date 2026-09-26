Device (URT0)
{
    Name (_HID, "SPRD0001")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70000000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x22 }
    })
}

Device (URT1)
{
    Name (_HID, "SPRD0001")
    Name (_UID, One)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x70100000, 0x00000100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x23 }
    })
}
