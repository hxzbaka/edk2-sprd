Device (ADI0)
{
    Name (_HID, "SPRD0003")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x32100000, 0x00100000)
    })
}

Device (PMI0)
{
    Name (_HID, "SPRD2730")
    Name (_UID, Zero)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
}
