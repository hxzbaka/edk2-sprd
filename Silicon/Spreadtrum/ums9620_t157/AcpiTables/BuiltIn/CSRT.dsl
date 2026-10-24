/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20230628 (64-bit version)
 * Copyright (c) 2000 - 2023 Intel Corporation
 * 
 * Disassembly of BuiltIn/CSRT.aml, Sat Sep 26 13:25:39 2026
 *
 * ACPI Data Table [CSRT]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue (in hex)
 */

[000h 0000 004h]                   Signature : "CSRT"    [Core System Resource Table]
[004h 0004 004h]                Table Length : 000000A0
[008h 0008 001h]                    Revision : 00
[009h 0009 001h]                    Checksum : 01
[00Ah 0010 006h]                      Oem ID : "SPRD  "
[010h 0016 008h]                Oem Table ID : "UMS512  "
[018h 0024 004h]                Oem Revision : 00000001
[01Ch 0028 004h]             Asl Compiler ID : "INTL"
[020h 0032 004h]       Asl Compiler Revision : 20260408


[024h 0036 004h]                      Length : 00000088
[028h 0040 004h]                   Vendor ID : 44525053
[02Ch 0044 004h]                Subvendor ID : 00000000
[030h 0048 002h]                   Device ID : 0001
[032h 0050 002h]                Subdevice ID : 0000
[034h 0052 002h]                    Revision : 0001
[036h 0054 002h]                    Reserved : 0000
[038h 0056 004h]          Shared Info Length : 0000001C

[03Ch 0060 002h]               Major Version : 0001
[03Eh 0062 002h]               Minor Version : 0000
[040h 0064 004h]       MMIO Base Address Low : 20200000
[044h 0068 004h]      MMIO Base Address High : 00000000
[048h 0072 004h]               GSI Interrupt : 000000FB
[04Ch 0076 001h]          Interrupt Polarity : 00
[04Dh 0077 001h]              Interrupt Mode : 00
[04Eh 0078 001h]                Num Channels : 1C
[04Fh 0079 001h]           DMA Address Width : 20
[050h 0080 002h]           Base Request Line : 0000
[052h 0082 002h]       Num Handshake Signals : 0000
[054h 0084 004h]              Max Block Size : 00004000

[058h 0088 004h]                      Length : 0000000C
[05Ch 0092 002h]                        Type : 0003
[05Eh 0094 002h]                     Subtype : 0001
[060h 0096 004h]                         UID : 41504D41

[064h 0100 004h]                      Length : 0000000C
[068h 0104 002h]                        Type : 0003
[06Ah 0106 002h]                     Subtype : 0000
[06Ch 0108 004h]                         UID : 30414843

[070h 0112 004h]                      Length : 0000000C
[074h 0116 002h]                        Type : 0003
[076h 0118 002h]                     Subtype : 0000
[078h 0120 004h]                         UID : 31414843

[07Ch 0124 004h]                      Length : 0000000C
[080h 0128 002h]                        Type : 0003
[082h 0130 002h]                     Subtype : 0000
[084h 0132 004h]                         UID : 32414843

[088h 0136 004h]                      Length : 0000000C
[08Ch 0140 002h]                        Type : 0003
[08Eh 0142 002h]                     Subtype : 0000
[090h 0144 004h]                         UID : 33414843

[094h 0148 004h]                      Length : 0000000C
[098h 0152 002h]                        Type : 0003
[09Ah 0154 002h]                     Subtype : 0000
[09Ch 0156 004h]                         UID : 34414843

Raw Table Data: Length 332 (0x14C)

    0000: 43 53 52 54 4C 01 00 00 00 01 53 50 52 44 20 20  // CSRTL.....SPRD  
    0010: 55 4D 53 35 31 32 20 20 01 00 00 00 49 4E 54 4C  // UMS512  ....INTL
    0020: 08 04 26 20 88 00 00 00 53 50 52 44 00 00 00 00  // ..& ....SPRD....
    0030: 01 00 00 00 01 00 00 00 1C 00 00 00 01 00 00 00  // ................
    0040: 00 00 00 20 00 00 00 00 38 00 00 00 00 00 1C 20  // ... ....8...... 
    0050: 00 00 00 00 00 40 00 00 0C 00 00 00 03 00 01 00  // .....@..........
    0060: 41 4D 50 41 0C 00 00 00 03 00 00 00 43 48 41 30  // AMPA........CHA0
    0070: 0C 00 00 00 03 00 00 00 43 48 41 31 0C 00 00 00  // ........CHA1....
    0080: 03 00 00 00 43 48 41 32 0C 00 00 00 03 00 00 00  // ....CHA2........
    0090: 43 48 41 33 0C 00 00 00 03 00 00 00 43 48 41 34  // CHA3........CHA4
    00A0: 0C 00 00 00 03 00 00 00 43 48 41 35 A0 00 00 00  // ........CHA5....
    00B0: 53 50 52 44 00 00 00 00 02 00 00 00 01 00 00 00  // SPRD............
    00C0: 1C 00 00 00 01 00 00 00 00 00 58 33 00 00 00 00  // ..........X3....
    00D0: D4 00 00 00 00 00 20 20 00 00 00 00 00 40 00 00  // ......  .....@..
    00E0: 0C 00 00 00 03 00 01 00 41 4D 41 47 0C 00 00 00  // ........AMAG....
    00F0: 03 00 00 00 43 48 41 30 0C 00 00 00 03 00 00 00  // ....CHA0........
    0100: 43 48 41 31 0C 00 00 00 03 00 00 00 43 48 41 32  // CHA1........CHA2
    0110: 0C 00 00 00 03 00 00 00 43 48 41 33 0C 00 00 00  // ........CHA3....
    0120: 03 00 00 00 43 48 41 34 0C 00 00 00 03 00 00 00  // ....CHA4........
    0130: 43 48 41 35 0C 00 00 00 03 00 00 00 43 48 41 36  // CHA5........CHA6
    0140: 0C 00 00 00 03 00 00 00 43 48 41 37              // ........CHA7