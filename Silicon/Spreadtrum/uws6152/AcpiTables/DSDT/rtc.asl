Scope (\_SB.PMI0)
{
    Device (RTC0)
    {
        Name (_HID, "SPRD0004")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00000200, 0x00000100)
        })
    }
}
