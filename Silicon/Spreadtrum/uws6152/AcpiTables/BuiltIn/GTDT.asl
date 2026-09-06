/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20250404 (64-bit version)
 * Copyright (c) 2000 - 2025 Intel Corporation
 *
 * Disassembly of BuiltIn/GTDT.aml
 *
 * ACPI Data Table [GTDT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue (in hex)
 */

[000h 0000 004h]                   Signature : "GTDT"    [Generic Timer Description Table]
[004h 0004 004h]                Table Length : 00000068
[008h 0008 001h]                    Revision : 03
[009h 0009 001h]                    Checksum : 77
[00Ah 0010 006h]                      Oem ID : "UNISOC"
[010h 0016 008h]                Oem Table ID : "UWS6152"
[018h 0024 004h]                Oem Revision : 00000001
[01Ch 0028 004h]             Asl Compiler ID : "INTL"
[020h 0032 004h]       Asl Compiler Revision : 20250404

[024h 0036 008h]       Counter Block Address : FFFFFFFFFFFFFFFF
[02Ch 0044 004h]                    Reserved : 00000000

[030h 0048 004h]        Secure EL1 Interrupt : 0000001D
[034h 0052 004h]   EL1 Flags (decoded below) : 00000000
                                Trigger Mode : 0
                                    Polarity : 0
                                   Always On : 0

[038h 0056 004h]    Non-Secure EL1 Interrupt : 0000001E
[03Ch 0060 004h]  NEL1 Flags (decoded below) : 00000000
                                Trigger Mode : 0
                                    Polarity : 0
                                   Always On : 0

[040h 0064 004h]     Virtual Timer Interrupt : 0000001B
[044h 0068 004h]    VT Flags (decoded below) : 00000000
                                Trigger Mode : 0
                                    Polarity : 0
                                   Always On : 0

[048h 0072 004h]    Non-Secure EL2 Interrupt : 0000001A
[04Ch 0076 004h]  NEL2 Flags (decoded below) : 00000000
                                Trigger Mode : 0
                                    Polarity : 0
                                   Always On : 0
[050h 0080 008h]  Counter Read Block Address : FFFFFFFFFFFFFFFF

[058h 0088 004h]        Platform Timer Count : 00000000
[05Ch 0092 004h]       Platform Timer Offset : 00000068
[060h 0096 004h]      Virtual EL2 Timer GSIV : 00000000
[064h 0100 004h]     Virtual EL2 Timer Flags : 00000000

Raw Table Data: Length 104 (0x68)

    0000: 47 54 44 54 68 00 00 00 03 77 55 4E 49 53 4F 43  // GTDTh....wUNISOC
    0010: 55 57 53 36 31 35 32 00 01 00 00 00 49 4E 54 4C  // UWS6152.....INTL
    0020: 04 04 25 20 FF FF FF FF FF FF FF FF 00 00 00 00  // ..% ............
    0030: 1D 00 00 00 00 00 00 00 1E 00 00 00 00 00 00 00  // ................
    0040: 1B 00 00 00 00 00 00 00 1A 00 00 00 00 00 00 00  // ................
    0050: FF FF FF FF FF FF FF FF 00 00 00 00 68 00 00 00  // ............h...
    0060: 00 00 00 00 00 00 00 00                          // ........
