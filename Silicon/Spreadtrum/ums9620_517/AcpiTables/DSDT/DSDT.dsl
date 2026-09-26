/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20230628 (64-bit version)
 * Copyright (c) 2000 - 2023 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of DSDT/DSDT.aml, Sat Sep 26 13:24:39 2026
 *
 * Original Table Header:
 *     Signature        "DSDT"
 *     Length           0x00002438 (9272)
 *     Revision         0x02
 *     Checksum         0x55
 *     OEM ID           "SPRDD "
 *     OEM Table ID     "UMS512  "
 *     OEM Revision     0x00000001 (1)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20260408 (539362312)
 */
DefinitionBlock ("", "DSDT", 2, "SPRDD ", "UMS512  ", 0x00000001)
{
    Scope (_SB)
    {
        Name (PSUB, "SPRDUSB0")
        Name (SOID, 0x0512)
        Name (SIDS, "UMS512")
        Name (SIDV, 0x00010000)
        Name (SVMJ, One)
        Name (SVMI, Zero)
        Name (SDFE, 0x0512)
        Name (SFES, "UMS512")
        Name (SIDM, Zero)
        Name (SOSN, Zero)
        Name (PLST, Zero)
        Name (WLEN, One)
        Name (TMK0, "UMS512-DSDT-r8")
        Device (SYSM)
        {
            Name (_HID, "ACPI0010" /* Processor Container Device */)  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Device (CL00)
            {
                Name (_HID, "ACPI0010" /* Processor Container Device */)  // _HID: Hardware ID
                Name (_UID, 0x10)  // _UID: Unique ID
                Name (_LPI, Package (0x05)  // _LPI: Low Power Idle States
                {
                    Zero, 
                    0x01000000, 
                    0x02, 
                    Package (0x0A)
                    {
                        0x012C, 
                        0x82, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero, 
                        0x20, 
                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        "L2_A55.D2d"
                    }, 

                    Package (0x0A)
                    {
                        0x1900, 
                        0x0384, 
                        One, 
                        Zero, 
                        Zero, 
                        0x03, 
                        0x40, 
                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        "L2_A55.D4"
                    }
                })
                Device (CPU0)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, Zero)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0190, 
                            0x64, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x01C2, 
                            0x012C, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C3"
                        }
                    })
                }

                Device (CPU1)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, One)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0190, 
                            0x64, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x01C2, 
                            0x012C, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C3"
                        }
                    })
                }

                Device (CPU2)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, 0x02)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0190, 
                            0x64, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x01C2, 
                            0x012C, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C3"
                        }
                    })
                }

                Device (CPU3)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, 0x03)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0190, 
                            0x64, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x01C2, 
                            0x012C, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C3"
                        }
                    })
                }

                Device (CPU4)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, 0x04)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0190, 
                            0x64, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x01C2, 
                            0x012C, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C3"
                        }
                    })
                }

                Device (CPU5)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, 0x05)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0190, 
                            0x64, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x01C2, 
                            0x012C, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A55.C3"
                        }
                    })
                }
            }

            Device (CL01)
            {
                Name (_HID, "ACPI0010" /* Processor Container Device */)  // _HID: Hardware ID
                Name (_UID, 0x20)  // _UID: Unique ID
                Name (_LPI, Package (0x05)  // _LPI: Low Power Idle States
                {
                    Zero, 
                    0x01000000, 
                    0x02, 
                    Package (0x0A)
                    {
                        0x0384, 
                        0xC8, 
                        Zero, 
                        Zero, 
                        Zero, 
                        Zero, 
                        0x20, 
                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        "L2_A75.D2d"
                    }, 

                    Package (0x0A)
                    {
                        0x1770, 
                        0x04B0, 
                        One, 
                        Zero, 
                        Zero, 
                        0x03, 
                        0x40, 
                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        ResourceTemplate ()
                        {
                            Register (SystemMemory, 
                                0x00,               // Bit Width
                                0x00,               // Bit Offset
                                0x0000000000000000, // Address
                                ,)
                        }, 

                        "L2_A75.D4"
                    }
                })
                Device (CPU6)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, 0x06)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A75.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0258, 
                            0x50, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A75.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x1B58, 
                            0x5A, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A75.C3"
                        }
                    })
                }

                Device (CPU7)
                {
                    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
                    Name (_UID, 0x07)  // _UID: Unique ID
                    Method (_STA, 0, NotSerialized)  // _STA: Status
                    {
                        Return (0x0F)
                    }

                    Name (_LPI, Package (0x06)  // _LPI: Low Power Idle States
                    {
                        Zero, 
                        Zero, 
                        0x03, 
                        Package (0x0A)
                        {
                            Zero, 
                            Zero, 
                            One, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x00000000FFFFFFFF, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A75.C1"
                        }, 

                        Package (0x0A)
                        {
                            0x0258, 
                            0x50, 
                            Zero, 
                            Zero, 
                            Zero, 
                            Zero, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000002, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A75.C2d"
                        }, 

                        Package (0x0A)
                        {
                            0x1B58, 
                            0x5A, 
                            One, 
                            One, 
                            Zero, 
                            0x03, 
                            ResourceTemplate ()
                            {
                                Register (FFixedHW, 
                                    0x20,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000040000003, // Address
                                    0x03,               // Access Size
                                    )
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            ResourceTemplate ()
                            {
                                Register (SystemMemory, 
                                    0x00,               // Bit Width
                                    0x00,               // Bit Offset
                                    0x0000000000000000, // Address
                                    ,)
                            }, 

                            "UMS512.A75.C3"
                        }
                    })
                }
            }
        }

        Device (URT0)
        {
            Name (_HID, "SPRD0001")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70000000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000022,
                }
            })
        }

        Device (URT1)
        {
            Name (_HID, "SPRD0001")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70100000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000023,
                }
            })
        }

        Device (EMMC)
        {
            Name (_HID, "SPRD7140")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, Zero)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
            {
                Return (Zero)
            }

            Device (SLOT)
            {
                Method (_ADR, 0, NotSerialized)  // _ADR: Address
                {
                    Return (0x08)
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
                {
                    Return (Zero)
                }
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x71400000,         // Address Base
                    0x00001000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000002A,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x25)
                {
                    Package (0x02)
                    {
                        "bus-width", 
                        0x08
                    }, 

                    Package (0x02)
                    {
                        "non-removable", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "cap-mmc-hw-reset", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "mmc-hs400-1_8v", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "mmc-hs400-enhanced-strobe", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "mmc-hs200-1_8v", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "mmc-ddr-1_8v", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "sprd,base-clock-khz", 
                        0x0005F370
                    }, 

                    Package (0x02)
                    {
                        "sprd,ap-clock-base", 
                        0x20200000
                    }, 

                    Package (0x02)
                    {
                        "sprd,ap-clock-length", 
                        0x1000
                    }, 

                    Package (0x02)
                    {
                        "sprd,ap-clock-mux-offset", 
                        0x90
                    }, 

                    Package (0x02)
                    {
                        "sprd,ap-clock-mux-mask", 
                        0x07
                    }, 

                    Package (0x02)
                    {
                        "sprd,ap-clock-mux-value", 
                        0x04
                    }, 

                    Package (0x02)
                    {
                        "sprd,apapb-gate-base", 
                        0x71000000
                    }, 

                    Package (0x02)
                    {
                        "sprd,apapb-gate-length", 
                        0x3000
                    }, 

                    Package (0x02)
                    {
                        "sprd,apapb-gate-offset", 
                        Zero
                    }, 

                    Package (0x02)
                    {
                        "sprd,apapb-gate-mask", 
                        0x02000000
                    }, 

                    Package (0x02)
                    {
                        "sprd,aon-gate-base", 
                        0x327D0000
                    }, 

                    Package (0x02)
                    {
                        "sprd,aon-gate-length", 
                        0x3000
                    }, 

                    Package (0x02)
                    {
                        "sprd,aon-gate-offset", 
                        0x013C
                    }, 

                    Package (0x02)
                    {
                        "sprd,aon-emmc-2x-mask", 
                        0x0100
                    }, 

                    Package (0x02)
                    {
                        "sprd,aon-emmc-1x-mask", 
                        0x0200
                    }, 

                    Package (0x02)
                    {
                        "sprd,pmu-gate-base", 
                        0x327E0000
                    }, 

                    Package (0x02)
                    {
                        "sprd,pmu-gate-length", 
                        0x3000
                    }, 

                    Package (0x02)
                    {
                        "sprd,pmu-rpll-gate-offset", 
                        0xAC
                    }, 

                    Package (0x02)
                    {
                        "sprd,pmu-rpll-gate-mask", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "sprd,gate-set-offset", 
                        0x1000
                    }, 

                    Package (0x02)
                    {
                        "sprd,gate-clear-offset", 
                        0x2000
                    }, 

                    Package (0x02)
                    {
                        "sprd,rpll-enable-delay-us", 
                        0x01E0
                    }, 

                    Package (0x02)
                    {
                        "sprd,adma-64bit", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "sprd,delay-mmchs", 
                        Package (0x04)
                        {
                            0x60, 
                            0xB7, 
                            0x34, 
                            0x34
                        }
                    }, 

                    Package (0x02)
                    {
                        "sprd,delay-ddr52", 
                        Package (0x04)
                        {
                            0x3E, 
                            0x37, 
                            0x24, 
                            0x24
                        }
                    }, 

                    Package (0x02)
                    {
                        "sprd,delay-hs200", 
                        Package (0x04)
                        {
                            0x7F, 
                            0xDE, 
                            0xD4, 
                            0xD4
                        }
                    }, 

                    Package (0x02)
                    {
                        "sprd,delay-hs400", 
                        Package (0x04)
                        {
                            0x3F, 
                            0xDF, 
                            0x2F, 
                            0x2F
                        }
                    }, 

                    Package (0x02)
                    {
                        "sprd,delay-hs400es", 
                        Package (0x04)
                        {
                            0x40, 
                            0x7F, 
                            0x2D, 
                            0x2D
                        }
                    }, 

                    Package (0x02)
                    {
                        "sprd,dll-auto-gate", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "sprd,allow-pio-fallback", 
                        One
                    }
                }
            })
        }

        Device (SDIO)
        {
            Name (_HID, "SPRD0002")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x71100000,         // Address Base
                    0x00001000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000033,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x03)
                {
                    Package (0x02)
                    {
                        "bus-width", 
                        0x04
                    }, 

                    Package (0x02)
                    {
                        "no-sdio", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "no-mmc", 
                        One
                    }
                }
            })
        }

        Device (WIFI)
        {
            Name (_HID, "SPRD0002")  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x71200000,         // Address Base
                    0x00001000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000034,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x03)
                {
                    Package (0x02)
                    {
                        "bus-width", 
                        0x04
                    }, 

                    Package (0x02)
                    {
                        "no-sd", 
                        One
                    }, 

                    Package (0x02)
                    {
                        "no-mmc", 
                        One
                    }
                }
            })
        }

        Device (I2C0)
        {
            Name (_HID, "SPRD0006")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70300000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000002B,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x00061A80
                    }
                }
            })
        }

        Device (I2C1)
        {
            Name (_HID, "SPRD0006")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70400000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000002C,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x00061A80
                    }
                }
            })
        }

        Device (I2C2)
        {
            Name (_HID, "SPRD0006")  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70500000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000002D,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x00061A80
                    }
                }
            })
        }

        Device (I2C3)
        {
            Name (_HID, "SPRD0006")  // _HID: Hardware ID
            Name (_UID, 0x03)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70600000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000002E,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x00061A80
                    }
                }
            })
        }

        Device (I2C4)
        {
            Name (_HID, "SPRD0006")  // _HID: Hardware ID
            Name (_UID, 0x04)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70700000,         // Address Base
                    0x00000100,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000002F,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "clock-frequency", 
                        0x00061A80
                    }
                }
            })
        }

        Device (SPI0)
        {
            Name (_HID, "SPRD0007")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x70800000,         // Address Base
                    0x00001000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000025,
                }
            })
        }

        Device (USB0)
        {
            Name (_HID, "SPRD0008")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x5FFF0000,         // Address Base
                    0x00002000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000065,
                }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x02)
                {
                    Package (0x02)
                    {
                        "dr-mode", 
                        "peripheral"
                    }, 

                    Package (0x02)
                    {
                        "wakeup-source", 
                        One
                    }
                }
            })
        }

        Device (ADI0)
        {
            Name (_HID, "SPRD0003")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x32100000,         // Address Base
                    0x00100000,         // Address Length
                    )
            })
        }

        Device (PMI0)
        {
            Name (_HID, "SPRD2730")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }
        }

        Device (EICD)
        {
            Name (_HID, "SPRD0009")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x32020000,         // Address Base
                    0x00000080,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x32030000,         // Address Base
                    0x00000080,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x32230000,         // Address Base
                    0x00000080,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x32270000,         // Address Base
                    0x00000080,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000068,
                }
            })
        }

        Device (EICL)
        {
            Name (_HID, "SPRD0009")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x32020080,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x32030080,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x32230080,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x32270080,         // Address Base
                    0x00000020,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000068,
                }
            })
        }

        Device (EICA)
        {
            Name (_HID, "SPRD0009")  // _HID: Hardware ID
            Name (_UID, 0x02)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x320200A0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x320300A0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x322300A0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x322700A0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000068,
                }
            })
        }

        Device (EICS)
        {
            Name (_HID, "SPRD0009")  // _HID: Hardware ID
            Name (_UID, 0x03)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x320200C0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x320300C0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x322300C0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x322700C0,         // Address Base
                    0x00000020,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x00000068,
                }
            })
        }

        Device (GPI0)
        {
            Name (_HID, "SPRD0011")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x32070000,         // Address Base
                    0x00010000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
                Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
                {
                    0x0000005D,
                }
            })
        }

        Scope (PMI0)
        {
            Device (PEIC)
            {
                Name (_HID, "SPRD0012")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (Zero)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00000280,         // Address Base
                        0x00000080,         // Address Length
                        )
                })
                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x01)
                    {
                        Package (0x02)
                        {
                            "pmic-local-irq", 
                            0x04
                        }
                    }
                })
            }
        }

        Device (TCM0)
        {
            Name (_HID, "VND0001")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                I2cSerialBusV2 (0x0022, ControllerInitiated, 0x00061A80,
                    AddressingMode7Bit, "\\_SB.I2C3",
                    0x00, ResourceConsumer, , Exclusive,
                    )
                GpioInt (Level, ActiveLow, Exclusive, PullDefault, 0x0000,
                    "\\_SB.GPI0", 0x00, ResourceConsumer, ,
                    )
                    {   // Pin list
                        0x0090
                    }
                GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                    "\\_SB.GPI0", 0x00, ResourceConsumer, ,
                    )
                    {   // Pin list
                        0x0091
                    }
            })
            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x03)
                {
                    Package (0x02)
                    {
                        "compatible", 
                        "synaptics,s3908"
                    }, 

                    Package (0x02)
                    {
                        "touchscreen-size-x", 
                        0x0438
                    }, 

                    Package (0x02)
                    {
                        "touchscreen-size-y", 
                        0x0924
                    }
                }
            })
        }

        Scope (PMI0)
        {
            Device (REGU)
            {
                Name (_HID, "SPRD0013")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "sprd,sc2730-regulator"
                        }, 

                        Package (0x02)
                        {
                            "regulator-names", 
                            Package (0x21)
                            {
                                "vddcpu", 
                                "vddgpu", 
                                "vddcore", 
                                "vddmodem", 
                                "vddmem", 
                                "vddmemq", 
                                "vddgen0", 
                                "vddgen1", 
                                "vddsram", 
                                "avdd18", 
                                "vddrf18", 
                                "vddcamio", 
                                "vddwcn", 
                                "vddcamd1", 
                                "vddcamd0", 
                                "vddrf1v25", 
                                "avdd12", 
                                "vddcama0", 
                                "vddcama1", 
                                "vddcammot", 
                                "vddsim1", 
                                "vddsim2", 
                                "vddemmccore", 
                                "vddsdcore", 
                                "vddsdio", 
                                "vdd28", 
                                "vddwifipa", 
                                "vdd18_dcxo", 
                                "vddusb33", 
                                "vddldo0", 
                                "vddldo1", 
                                "vddldo2", 
                                "vddkpled"
                            }
                        }
                    }
                })
            }
        }

        Device (DPU0)
        {
            Name (_HID, "SPRD0014")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x20300000,         // Address Base
                    0x00001000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000003B,
                }
            })
        }

        Device (GSP0)
        {
            Name (_HID, "SPRD0015")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x20300000,         // Address Base
                    0x00002000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000003A,
                }
            })
        }

        Device (DIOM)
        {
            Name (_HID, "SPRD0016")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x20300000,         // Address Base
                    0x00000800,         // Address Length
                    )
                Memory32Fixed (ReadWrite,
                    0x20300800,         // Address Base
                    0x00000080,         // Address Length
                    )
            })
        }

        Device (DSI0)
        {
            Name (_HID, "SPRD0017")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x20400000,         // Address Base
                    0x00001000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x0000003E,
                    0x0000003F,
                }
            })
        }

        Device (DPHY)
        {
            Name (_HID, "SPRD0018")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x20400000,         // Address Base
                    0x00001000,         // Address Length
                    )
            })
        }

        Device (FBR0)
        {
            Name (_HID, "SPRD0019")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x9EB0C000,         // Address Base
                    0x0012D000,         // Address Length
                    )
            })
        }

        Device (VBC0)
        {
            Name (_HID, "SPRD0020")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x33480000,         // Address Base
                    0x00001000,         // Address Length
                    )
            })
        }

        Device (MCDT)
        {
            Name (_HID, "SPRD0021")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x33490000,         // Address Base
                    0x00000170,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x000000D6,
                }
            })
        }

        Device (ADMA)
        {
            Name (_HID, "SPRD0022")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x33580000,         // Address Base
                    0x00004000,         // Address Length
                    )
                Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                {
                    0x000000D4,
                }
            })
        }

        Device (DCOD)
        {
            Name (_HID, "SPRD0023")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_CCA, One)  // _CCA: Cache Coherency Attribute
            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }

            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                Memory32Fixed (ReadWrite,
                    0x33750000,         // Address Base
                    0x00001000,         // Address Length
                    )
            })
        }

        Scope (PMI0)
        {
            Device (ADC0)
            {
                Name (_HID, "SPRD0024")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00000504,         // Address Base
                        0x00000100,         // Address Length
                        )
                })
                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x01)
                    {
                        Package (0x02)
                        {
                            "pmic-local-irq", 
                            Zero
                        }
                    }
                })
            }

            Device (FGU0)
            {
                Name (_HID, "SPRD0025")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00000C00,         // Address Base
                        0x00000100,         // Address Length
                        )
                })
                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x01)
                    {
                        Package (0x02)
                        {
                            "pmic-local-irq", 
                            0x03
                        }
                    }
                })
            }

            Device (ACOD)
            {
                Name (_HID, "SPRD0026")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00001000,         // Address Base
                        0x00000100,         // Address Length
                        )
                })
                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x01)
                    {
                        Package (0x02)
                        {
                            "sprd,fixed-sample-rate", 
                            0xBB80
                        }
                    }
                })
            }

            Device (VIBR)
            {
                Name (_HID, "SPRD0027")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00001B90,         // Address Base
                        0x00000010,         // Address Length
                        )
                })
            }
        }

        Scope (I2C1)
        {
            Device (CAM0)
            {
                Name (_HID, "SPRD0028")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "sensor-main"
                        }, 

                        Package (0x02)
                        {
                            "reg", 
                            0x6C
                        }
                    }
                })
            }
        }

        Scope (I2C3)
        {
            Device (TPD0)
            {
                Name (_HID, "SPRD0029")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "hynitron"
                        }, 

                        Package (0x02)
                        {
                            "reg", 
                            0x1A
                        }
                    }
                })
            }
        }

        Scope (I2C4)
        {
            Device (ACC0)
            {
                Name (_HID, "SPRD0030")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "da,da217"
                        }, 

                        Package (0x02)
                        {
                            "reg", 
                            0x26
                        }
                    }
                })
            }

            Device (HRS0)
            {
                Name (_HID, "SPRD0031")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "bd166x_hr"
                        }, 

                        Package (0x02)
                        {
                            "reg", 
                            0x38
                        }
                    }
                })
            }

            Device (HALL)
            {
                Name (_HID, "SPRD0032")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "och1970"
                        }, 

                        Package (0x02)
                        {
                            "reg", 
                            0x0D
                        }
                    }
                })
            }

            Device (CHG0)
            {
                Name (_HID, "SPRD0033")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
                {
                    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                    Package (0x02)
                    {
                        Package (0x02)
                        {
                            "compatible", 
                            "fairchild,fan54015_chg"
                        }, 

                        Package (0x02)
                        {
                            "reg", 
                            0x6A
                        }
                    }
                })
            }
        }

        Scope (PMI0)
        {
            Device (RTC0)
            {
                Name (_HID, "SPRD0004")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00000200,         // Address Base
                        0x00000100,         // Address Length
                        )
                })
            }
        }

        Scope (PMI0)
        {
            Device (PWR0)
            {
                Name (_HID, "SPRD0005")  // _HID: Hardware ID
                Name (_UID, Zero)  // _UID: Unique ID
                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
                    Return (0x0F)
                }

                Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
                {
                    Memory32Fixed (ReadWrite,
                        0x00001820,         // Address Base
                        0x00000100,         // Address Length
                        )
                })
            }
        }
    }
}

