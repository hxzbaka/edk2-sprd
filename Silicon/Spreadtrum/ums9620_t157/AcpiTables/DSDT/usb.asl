Device (USB0)
{
    Name (_HID, "SPRD0008")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x5FFF0000, 0x00002000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x65 }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "dr-mode", "peripheral" },
            Package () { "wakeup-source", One }
        }
    })
}
