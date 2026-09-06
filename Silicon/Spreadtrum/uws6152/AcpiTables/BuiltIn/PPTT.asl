/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20250404 (64-bit version)
 * Copyright (c) 2000 - 2025 Intel Corporation
 *
 * Disassembly of BuiltIn/PPTT.aml
 *
 * ACPI Data Table [PPTT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue (in hex)
 */

[000h 0000 004h]                   Signature : "PPTT"    [Processor Properties Topology Table]
[004h 0004 004h]                Table Length : 000000B0
[008h 0008 001h]                    Revision : 03
[009h 0009 001h]                    Checksum : EB
[00Ah 0010 006h]                      Oem ID : "UNISOC"
[010h 0016 008h]                Oem Table ID : "UWS6152"
[018h 0024 004h]                Oem Revision : 00000001
[01Ch 0028 004h]             Asl Compiler ID : "INTL"
[020h 0032 004h]       Asl Compiler Revision : 20250404


[024h 0036 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[025h 0037 001h]                      Length : 14
[026h 0038 002h]                    Reserved : 0000
[028h 0040 004h]       Flags (decoded below) : 00000001
                            Physical package : 1
                     ACPI Processor ID valid : 0
                       Processor is a thread : 0
                              Node is a leaf : 0
                    Identical Implementation : 0
[02Ch 0044 004h]                      Parent : 00000000
[030h 0048 004h]           ACPI Processor ID : 00000000
[034h 0052 004h]     Private Resource Number : 00000000

[038h 0056 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[039h 0057 001h]                      Length : 14
[03Ah 0058 002h]                    Reserved : 0000
[03Ch 0060 004h]       Flags (decoded below) : 00000000
                            Physical package : 0
                     ACPI Processor ID valid : 0
                       Processor is a thread : 0
                              Node is a leaf : 0
                    Identical Implementation : 0
[040h 0064 004h]                      Parent : 00000024
[044h 0068 004h]           ACPI Processor ID : 00000000
[048h 0072 004h]     Private Resource Number : 00000000

[04Ch 0076 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[04Dh 0077 001h]                      Length : 14
[04Eh 0078 002h]                    Reserved : 0000
[050h 0080 004h]       Flags (decoded below) : 00000000
                            Physical package : 0
                     ACPI Processor ID valid : 0
                       Processor is a thread : 0
                              Node is a leaf : 0
                    Identical Implementation : 0
[054h 0084 004h]                      Parent : 00000024
[058h 0088 004h]           ACPI Processor ID : 00000000
[05Ch 0092 004h]     Private Resource Number : 00000000

[060h 0096 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[061h 0097 001h]                      Length : 14
[062h 0098 002h]                    Reserved : 0000
[064h 0100 004h]       Flags (decoded below) : 0000000A
                            Physical package : 0
                     ACPI Processor ID valid : 1
                       Processor is a thread : 0
                              Node is a leaf : 1
                    Identical Implementation : 0
[068h 0104 004h]                      Parent : 00000038
[06Ch 0108 004h]           ACPI Processor ID : 00000000
[070h 0112 004h]     Private Resource Number : 00000000

[074h 0116 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[075h 0117 001h]                      Length : 14
[076h 0118 002h]                    Reserved : 0000
[078h 0120 004h]       Flags (decoded below) : 0000000A
                            Physical package : 0
                     ACPI Processor ID valid : 1
                       Processor is a thread : 0
                              Node is a leaf : 1
                    Identical Implementation : 0
[07Ch 0124 004h]                      Parent : 00000038
[080h 0128 004h]           ACPI Processor ID : 00000001
[084h 0132 004h]     Private Resource Number : 00000000

[088h 0136 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[089h 0137 001h]                      Length : 14
[08Ah 0138 002h]                    Reserved : 0000
[08Ch 0140 004h]       Flags (decoded below) : 0000000A
                            Physical package : 0
                     ACPI Processor ID valid : 1
                       Processor is a thread : 0
                              Node is a leaf : 1
                    Identical Implementation : 0
[090h 0144 004h]                      Parent : 00000038
[094h 0148 004h]           ACPI Processor ID : 00000002
[098h 0152 004h]     Private Resource Number : 00000000

[09Ch 0156 001h]               Subtable Type : 00 [Processor Hierarchy Node]
[09Dh 0157 001h]                      Length : 14
[09Eh 0158 002h]                    Reserved : 0000
[0A0h 0160 004h]       Flags (decoded below) : 0000000A
                            Physical package : 0
                     ACPI Processor ID valid : 1
                       Processor is a thread : 0
                              Node is a leaf : 1
                    Identical Implementation : 0
[0A4h 0164 004h]                      Parent : 0000004C
[0A8h 0168 004h]           ACPI Processor ID : 00000003
[0ACh 0172 004h]     Private Resource Number : 00000000

Raw Table Data: Length 176 (0xB0)

    0000: 50 50 54 54 B0 00 00 00 03 EB 55 4E 49 53 4F 43  // PPTT......UNISOC
    0010: 55 57 53 36 31 35 32 00 01 00 00 00 49 4E 54 4C  // UWS6152.....INTL
    0020: 04 04 25 20 00 14 00 00 01 00 00 00 00 00 00 00  // ..% ............
    0030: 00 00 00 00 00 00 00 00 00 14 00 00 00 00 00 00  // ................
    0040: 24 00 00 00 00 00 00 00 00 00 00 00 00 14 00 00  // $...............
    0050: 00 00 00 00 24 00 00 00 00 00 00 00 00 00 00 00  // ....$...........
    0060: 00 14 00 00 0A 00 00 00 38 00 00 00 00 00 00 00  // ........8.......
    0070: 00 00 00 00 00 14 00 00 0A 00 00 00 38 00 00 00  // ............8...
    0080: 01 00 00 00 00 00 00 00 00 14 00 00 0A 00 00 00  // ................
    0090: 38 00 00 00 02 00 00 00 00 00 00 00 00 14 00 00  // 8...............
    00A0: 0A 00 00 00 4C 00 00 00 03 00 00 00 00 00 00 00  // ....L...........
