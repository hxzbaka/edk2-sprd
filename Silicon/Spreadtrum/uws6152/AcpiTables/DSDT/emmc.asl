Device (EMMC)
{
    Name (_HID, "SPRD7140")
    Name (_UID, Zero)
    Name (_CCA, One)
    Method (_STA, 0, NotSerialized)
    {
        Return (0x0F)
    }
    Name (_CRS, ResourceTemplate ()
    {
        Memory32Fixed (ReadWrite, 0x71400000, 0x00001000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 0x2A }
    })
    Name (_DSD, Package ()
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
            Package () { "bus-width", 0x08 },
            Package () { "non-removable", One },
            Package () { "cap-mmc-hw-reset", One },
            Package () { "mmc-hs400-1_8v", One },
            Package () { "mmc-hs400-enhanced-strobe", One },
            Package () { "mmc-hs200-1_8v", One },
            Package () { "mmc-ddr-1_8v", One },
            Package () { "sprd,base-clock-khz", 390000 },
            Package () { "sprd,ap-clock-base", 0x20200000 },
            Package () { "sprd,ap-clock-length", 0x00001000 },
            Package () { "sprd,ap-clock-mux-offset", 0x0000008C },
            Package () { "sprd,ap-clock-mux-mask", 0x00000007 },
            Package () { "sprd,ap-clock-mux-value", 0x00000004 },
            Package () { "sprd,apapb-gate-base", 0x71000000 },
            Package () { "sprd,apapb-gate-length", 0x00003000 },
            Package () { "sprd,apapb-gate-offset", Zero },
            Package () { "sprd,apapb-gate-mask", 0x02000000 },
            Package () { "sprd,aon-gate-base", 0x327D0000 },
            Package () { "sprd,aon-gate-length", 0x00003000 },
            Package () { "sprd,aon-gate-offset", 0x0000013C },
            Package () { "sprd,aon-emmc-2x-mask", 0x00000100 },
            Package () { "sprd,aon-emmc-1x-mask", 0x00000200 },
            Package () { "sprd,pmu-gate-base", 0x327E0000 },
            Package () { "sprd,pmu-gate-length", 0x00003000 },
            Package () { "sprd,pmu-rpll-gate-offset", 0x000000AC },
            Package () { "sprd,pmu-rpll-gate-mask", One },
            Package () { "sprd,gate-set-offset", 0x00001000 },
            Package () { "sprd,gate-clear-offset", 0x00002000 },
            Package () { "sprd,rpll-enable-delay-us", 480 },
            Package () { "sprd,adma-64bit", One },
            Package () { "sprd,dma-address-bits", 64 },
            Package () { "sprd,delay-legacy", Package () { 0x00, 0x00, 0x00, 0x00 } },
            Package () { "sprd,delay-mmchs", Package () { 0x7F, 0x40, 0x40, 0x40 } },
            Package () { "sprd,delay-ddr52", Package () { 0x30, 0x27, 0x19, 0x19 } },
            Package () { "sprd,delay-hs200", Package () { 0x7F, 0xDE, 0xCA, 0xCA } },
            Package () { "sprd,delay-hs400", Package () { 0x4B, 0x72, 0x38, 0x38 } },
            Package () { "sprd,delay-hs400es", Package () { 0x4F, 0x7F, 0x32, 0x32 } },
            Package () { "sprd,dll-auto-gate", One },
            Package () { "sprd,allow-pio-fallback", One }
        }
    })
}
