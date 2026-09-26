Scope (\_SB.PMI0)
{
    Device (PWR0)
    {
        Name (_HID, "SPRD0005")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00001820, 0x00000100)
        })
    }
}
