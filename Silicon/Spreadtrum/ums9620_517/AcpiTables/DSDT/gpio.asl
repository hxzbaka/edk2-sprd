Device (EICD)
{
    Name (_HID, "SPRD0009")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x32020000, 0x00000080)
        Memory32Fixed (ReadWrite, 0x32030000, 0x00000080)
        Memory32Fixed (ReadWrite, 0x32230000, 0x00000080)
        Memory32Fixed (ReadWrite, 0x32270000, 0x00000080)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x68 }
    })
}

Device (EICL)
{
    Name (_HID, "SPRD0009")
    Name (_UID, One)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x32020080, 0x00000020)
        Memory32Fixed (ReadWrite, 0x32030080, 0x00000020)
        Memory32Fixed (ReadWrite, 0x32230080, 0x00000020)
        Memory32Fixed (ReadWrite, 0x32270080, 0x00000020)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x68 }
    })
}

Device (EICA)
{
    Name (_HID, "SPRD0009")
    Name (_UID, 0x02)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x320200A0, 0x00000020)
        Memory32Fixed (ReadWrite, 0x320300A0, 0x00000020)
        Memory32Fixed (ReadWrite, 0x322300A0, 0x00000020)
        Memory32Fixed (ReadWrite, 0x322700A0, 0x00000020)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x68 }
    })
}

Device (EICS)
{
    Name (_HID, "SPRD0009")
    Name (_UID, 0x03)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x320200C0, 0x00000020)
        Memory32Fixed (ReadWrite, 0x320300C0, 0x00000020)
        Memory32Fixed (ReadWrite, 0x322300C0, 0x00000020)
        Memory32Fixed (ReadWrite, 0x322700C0, 0x00000020)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x68 }
    })
}

Device (GPI0)
{
    Name (_HID, "SPRD0011")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x32070000, 0x00010000)

        // GpioClx binds one interrupt resource to each AP GPIO bank.  The
        // UMS512 hardware routes all 16 banks through GIC SPI 61 (GSI 0x5D),
        // so the same parent interrupt is described once for every bank.
        // The descriptors must be shared because they use the same GIC line.
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
        Interrupt (ResourceConsumer, Level, ActiveHigh, Shared) { 0x5D }
    })
}

Scope (\_SB.PMI0)
{
    Device (PEIC)
    {
        Name (_HID, "SPRD0012")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            // The EIC register address is an SC2730 ADI offset, not CPU MMIO.
            // Keep it hidden until a PMIC/ADI-backed EIC controller exists.
            Return (Zero)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00000280, 0x00000080)
        })
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "pmic-local-irq", 0x04 }
            }
        })
    }
}
