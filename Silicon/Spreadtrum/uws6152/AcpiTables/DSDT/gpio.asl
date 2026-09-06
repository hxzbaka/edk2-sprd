Device (GPIO)
{
    Name (_HID, "SPRD6150")
    Name (_UID, Zero)
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x32070000, 0x00010000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Memory32Fixed (ReadWrite, 0x32450288, 0x00000008)
    })
}
