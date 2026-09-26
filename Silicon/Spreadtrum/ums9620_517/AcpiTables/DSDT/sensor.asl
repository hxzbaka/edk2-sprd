Scope (\_SB.I2C1)
{
    Device (CAM0)
    {
        Name (_HID, "SPRD0028")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "compatible", "sensor-main" },
                Package () { "reg", 0x6C }
            }
        })
    }
}

Scope (\_SB.I2C3)
{
    Device (TPD0)
    {
        Name (_HID, "SPRD0029")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "compatible", "hynitron" },
                Package () { "reg", 0x1A }
            }
        })
    }
}

Scope (\_SB.I2C4)
{
    Device (ACC0)
    {
        Name (_HID, "SPRD0030")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "compatible", "da,da217" },
                Package () { "reg", 0x26 }
            }
        })
    }

    Device (HRS0)
    {
        Name (_HID, "SPRD0031")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "compatible", "bd166x_hr" },
                Package () { "reg", 0x38 }
            }
        })
    }

    Device (HALL)
    {
        Name (_HID, "SPRD0032")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "compatible", "och1970" },
                Package () { "reg", 0x0D }
            }
        })
    }

    Device (CHG0)
    {
        Name (_HID, "SPRD0033")
        Name (_UID, Zero)
        Method (_STA, 0, NotSerialized)
        {
            Return (0x0F)
        }
        Name (_DSD, Package ()
        {
            ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package ()
            {
                Package () { "compatible", "fairchild,fan54015_chg" },
                Package () { "reg", 0x6A }
            }
        })
    }
}
