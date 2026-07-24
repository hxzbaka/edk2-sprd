Device (VBC0)
{
    Name (_HID, "SPRD0020")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x33480000, 0x00001000)
    })
}

Device (MCDT)
{
    Name (_HID, "SPRD0021")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x33490000, 0x00000170)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0xD6 }
    })
}

Device (ADMA)
{
    Name (_HID, "SPRD0022")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x33580000, 0x00004000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0xD4 }
    })
}

Device (DCOD)
{
    Name (_HID, "SPRD0023")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x33750000, 0x00001000)
    })
}

Scope (\_SB.PMI0)
{
    Device (ADC0)
    {
        Name (_HID, "SPRD0024")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00000504, 0x00000100)
        })
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "pmic-local-irq", Zero }
            }
        })
    }

    Device (FGU0)
    {
        Name (_HID, "SPRD0025")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00000C00, 0x00000100)
        })
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "pmic-local-irq", 0x03 }
            }
        })
    }

    Device (ACOD)
    {
        Name (_HID, "SPRD0026")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00001000, 0x00000100)
        })
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "sprd,fixed-sample-rate", 48000 }
            }
        })
    }

    Device (VIBR)
    {
        Name (_HID, "SPRD0027")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_CRS, ResourceTemplate ()
        {
            Memory32Fixed (ReadWrite, 0x00001B90, 0x00000010)
        })
    }
}
