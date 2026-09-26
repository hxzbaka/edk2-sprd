Scope (\_SB.PMI0)
{
    Device (REGU)
    {
        Name (_HID, "SPRD0013")
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
                Package () { "compatible", "sprd,sc2730-regulator" },
                Package () { "regulator-names", Package () { "vddcpu", "vddgpu", "vddcore", "vddmodem", "vddmem", "vddmemq", "vddgen0", "vddgen1", "vddsram", "avdd18", "vddrf18", "vddcamio", "vddwcn", "vddcamd1", "vddcamd0", "vddrf1v25", "avdd12", "vddcama0", "vddcama1", "vddcammot", "vddsim1", "vddsim2", "vddemmccore", "vddsdcore", "vddsdio", "vdd28", "vddwifipa", "vdd18_dcxo", "vddusb33", "vddldo0", "vddldo1", "vddldo2", "vddkpled" } }
            }
        })
    }
}
