Device (I2C3)
{
    Name (_HID, "SPRD6151")
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
        Memory32Fixed (ReadWrite, 0x71001000, 0x00000004)
        Memory32Fixed (ReadWrite, 0x71002000, 0x00000004)
        Memory32Fixed (ReadWrite, 0x20200040, 0x00000004)
        Memory32Fixed (ReadWrite, 0x32450290, 0x00000008)
    })
}