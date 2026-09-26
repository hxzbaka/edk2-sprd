// Synaptics S3908 TCM touchscreen on UMS512 I2C3.
Device (TCM0)
{
    Name (_HID, "VND0001")
    Name (_UID, Zero)

    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }

    Name (_CRS, ResourceTemplate ()
    {
        I2cSerialBusV2 (
            0x0022,
            ControllerInitiated,
            400000,
            AddressingMode7Bit,
            "\\_SB.I2C3",
            0x00,
            ResourceConsumer,
            ,
            Exclusive
        )

        GpioInt (
            Level,
            ActiveLow,
            Exclusive,
            PullDefault,
            0x0000,
            "\\_SB.GPI0",
            0x00,
            ResourceConsumer,
            ,
        )
        {
            144
        }

        GpioIo (
            Exclusive,
            PullDefault,
            0x0000,
            0x0000,
            IoRestrictionOutputOnly,
            "\\_SB.GPI0",
            0x00,
            ResourceConsumer,
            ,
        )
        {
            145
        }

    })

    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "compatible", "synaptics,s3908" },
            Package () { "touchscreen-size-x", 1080 },
            Package () { "touchscreen-size-y", 2340 }
        }
    })
}
