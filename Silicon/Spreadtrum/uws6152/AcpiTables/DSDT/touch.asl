Device (TPD0)
{
    Name (_HID, "HYNT9217")
    Name (_UID, Zero)
    Name (_STA, 0x0F)
    Name (_DEP, Package () { \_SB.GPIO, \_SB.I2C3 })
    Name (_CRS, ResourceTemplate ()
    {
        I2cSerialBusV2 (0x005A, ControllerInitiated, 400000,
            AddressingMode7Bit, "\\_SB.I2C3", 0x00,
            ResourceConsumer, , Exclusive)
        GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly,
            "\\_SB.GPIO", 0, ResourceConsumer, , ) { 145 }
        GpioInt (Edge, ActiveHigh, Exclusive, PullDefault, 0,
            "\\_SB.GPIO", 0, ResourceConsumer, , ) { 144 }
    })
}
