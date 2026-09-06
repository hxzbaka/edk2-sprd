/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20250404 (64-bit version)
 * Copyright (c) 2000 - 2025 Intel Corporation
 *
 * Disassembly of BuiltIn/SPCR.aml
 *
 * ACPI Data Table [SPCR]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue (in hex)
 */

[000h 0000 004h]                   Signature : "SPCR"    [Serial Port Console Redirection Table]
[004h 0004 004h]                Table Length : 0000005A
[008h 0008 001h]                    Revision : 02
[009h 0009 001h]                    Checksum : F7
[00Ah 0010 006h]                      Oem ID : "UNISOC"
[010h 0016 008h]                Oem Table ID : "UWS6152"
[018h 0024 004h]                Oem Revision : 00000001
[01Ch 0028 004h]             Asl Compiler ID : "INTL"
[020h 0032 004h]       Asl Compiler Revision : 20250404

[024h 0036 001h]              Interface Type : 12
[025h 0037 003h]                    Reserved : 000000

[028h 0040 00Ch]        Serial Port Register : [Generic Address Structure]
[028h 0040 001h]                    Space ID : 00 [SystemMemory]
[029h 0041 001h]                   Bit Width : 20
[02Ah 0042 001h]                  Bit Offset : 00
[02Bh 0043 001h]        Encoded Access Width : 03 [DWord Access:32]
[02Ch 0044 008h]                     Address : 0000000070100000

[034h 0052 001h]              Interrupt Type : 08
[035h 0053 001h]         PCAT-compatible IRQ : 00
[036h 0054 004h]                   Interrupt : 00000003
[03Ah 0058 001h]                   Baud Rate : 07
[03Bh 0059 001h]                      Parity : 00
[03Ch 0060 001h]                   Stop Bits : 01
[03Dh 0061 001h]                Flow Control : 00
[03Eh 0062 001h]               Terminal Type : 02
[03Fh 0063 001h]                    Language : 00
[040h 0064 002h]               PCI Device ID : 0000
[042h 0066 002h]               PCI Vendor ID : 0000
[044h 0068 001h]                     PCI Bus : 00
[045h 0069 001h]                  PCI Device : 00
[046h 0070 001h]                PCI Function : 00
[047h 0071 004h]                   PCI Flags : 00000000
[04Bh 0075 001h]                 PCI Segment : 00
[04Ch 0076 004h]             Uart Clock Freq : 00000000
[050h 0080 004h]           Precise Baud rate : 00000000
[054h 0084 002h]       NameSpaceStringLength : 0002
[056h 0086 002h]       NameSpaceStringOffset : 0058
[058h 0088 002h]             NamespaceString : "."

Raw Table Data: Length 90 (0x5A)

    0000: 53 50 43 52 5A 00 00 00 02 F7 55 4E 49 53 4F 43  // SPCRZ.....UNISOC
    0010: 55 57 53 36 31 35 32 00 01 00 00 00 49 4E 54 4C  // UWS6152.....INTL
    0020: 04 04 25 20 12 00 00 00 00 20 00 03 00 00 10 70  // ..% ..... .....p
    0030: 00 00 00 00 08 00 03 00 00 00 07 00 01 00 02 00  // ................
    0040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0050: 00 00 00 00 02 00 58 00 2E 00                    // ......X...
