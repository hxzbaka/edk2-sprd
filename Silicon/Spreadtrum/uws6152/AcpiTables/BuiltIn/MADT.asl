/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20250404 (64-bit version)
 * Copyright (c) 2000 - 2025 Intel Corporation
 *
 * Disassembly of BuiltIn/MADT.aml
 *
 * ACPI Data Table [APIC]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue (in hex)
 */

[000h 0000 004h]                   Signature : "APIC"    [Multiple APIC Description Table (MADT)]
[004h 0004 004h]                Table Length : 00000194
[008h 0008 001h]                    Revision : 05
[009h 0009 001h]                    Checksum : 6E
[00Ah 0010 006h]                      Oem ID : "UNISOC"
[010h 0016 008h]                Oem Table ID : "UWS6152"
[018h 0024 004h]                Oem Revision : 00000001
[01Ch 0028 004h]             Asl Compiler ID : "INTL"
[020h 0032 004h]       Asl Compiler Revision : 20250404

[024h 0036 004h]          Local Apic Address : 00000000
[028h 0040 004h]       Flags (decoded below) : 00000000
                         PC-AT Compatibility : 0

[02Ch 0044 001h]               Subtable Type : 0B [Generic Interrupt Controller]
[02Dh 0045 001h]                      Length : 50
[02Eh 0046 002h]                    Reserved : 0000
[030h 0048 004h]        CPU Interface Number : 00000000
[034h 0052 004h]               Processor UID : 00000000
[038h 0056 004h]       Flags (decoded below) : 00000001
                           Processor Enabled : 1
          Performance Interrupt Trigger Mode : 0
          Virtual GIC Interrupt Trigger Mode : 0
[03Ch 0060 004h]    Parking Protocol Version : 00000000
[040h 0064 004h]       Performance Interrupt : 00000070
[044h 0068 008h]              Parked Address : 0000000000000000
[04Ch 0076 008h]                Base Address : 0000000000000000
[054h 0084 008h]    Virtual GIC Base Address : 0000000000000000
[05Ch 0092 008h] Hypervisor GIC Base Address : 0000000000000000
[064h 0100 004h]       Virtual GIC Interrupt : 00000000
[068h 0104 008h]  Redistributor Base Address : 0000000000000000
[070h 0112 008h]                   ARM MPIDR : 0000000000000000
[078h 0120 001h]            Efficiency Class : 00
[079h 0121 001h]                    Reserved : 00
[07Ah 0122 002h]      SPE Overflow Interrupt : 0000
[07Ch 0124 002h]              TRBE Interrupt : 500B

[07Ch 0124 001h]               Subtable Type : 0B [Generic Interrupt Controller]
[07Dh 0125 001h]                      Length : 50
[07Eh 0126 002h]                    Reserved : 0000
[080h 0128 004h]        CPU Interface Number : 00000000
[084h 0132 004h]               Processor UID : 00000001
[088h 0136 004h]       Flags (decoded below) : 00000001
                           Processor Enabled : 1
          Performance Interrupt Trigger Mode : 0
          Virtual GIC Interrupt Trigger Mode : 0
[08Ch 0140 004h]    Parking Protocol Version : 00000000
[090h 0144 004h]       Performance Interrupt : 00000071
[094h 0148 008h]              Parked Address : 0000000000000000
[09Ch 0156 008h]                Base Address : 0000000000000000
[0A4h 0164 008h]    Virtual GIC Base Address : 0000000000000000
[0ACh 0172 008h] Hypervisor GIC Base Address : 0000000000000000
[0B4h 0180 004h]       Virtual GIC Interrupt : 00000000
[0B8h 0184 008h]  Redistributor Base Address : 0000000000000000
[0C0h 0192 008h]                   ARM MPIDR : 0000000000000100
[0C8h 0200 001h]            Efficiency Class : 00
[0C9h 0201 001h]                    Reserved : 00
[0CAh 0202 002h]      SPE Overflow Interrupt : 0000
[0CCh 0204 002h]              TRBE Interrupt : 500B

[0CCh 0204 001h]               Subtable Type : 0B [Generic Interrupt Controller]
[0CDh 0205 001h]                      Length : 50
[0CEh 0206 002h]                    Reserved : 0000
[0D0h 0208 004h]        CPU Interface Number : 00000000
[0D4h 0212 004h]               Processor UID : 00000002
[0D8h 0216 004h]       Flags (decoded below) : 00000001
                           Processor Enabled : 1
          Performance Interrupt Trigger Mode : 0
          Virtual GIC Interrupt Trigger Mode : 0
[0DCh 0220 004h]    Parking Protocol Version : 00000000
[0E0h 0224 004h]       Performance Interrupt : 00000072
[0E4h 0228 008h]              Parked Address : 0000000000000000
[0ECh 0236 008h]                Base Address : 0000000000000000
[0F4h 0244 008h]    Virtual GIC Base Address : 0000000000000000
[0FCh 0252 008h] Hypervisor GIC Base Address : 0000000000000000
[104h 0260 004h]       Virtual GIC Interrupt : 00000000
[108h 0264 008h]  Redistributor Base Address : 0000000000000000
[110h 0272 008h]                   ARM MPIDR : 0000000000000200
[118h 0280 001h]            Efficiency Class : 00
[119h 0281 001h]                    Reserved : 00
[11Ah 0282 002h]      SPE Overflow Interrupt : 0000
[11Ch 0284 002h]              TRBE Interrupt : 500B

[11Ch 0284 001h]               Subtable Type : 0B [Generic Interrupt Controller]
[11Dh 0285 001h]                      Length : 50
[11Eh 0286 002h]                    Reserved : 0000
[120h 0288 004h]        CPU Interface Number : 00000000
[124h 0292 004h]               Processor UID : 00000003
[128h 0296 004h]       Flags (decoded below) : 00000001
                           Processor Enabled : 1
          Performance Interrupt Trigger Mode : 0
          Virtual GIC Interrupt Trigger Mode : 0
[12Ch 0300 004h]    Parking Protocol Version : 00000000
[130h 0304 004h]       Performance Interrupt : 00000073
[134h 0308 008h]              Parked Address : 0000000000000000
[13Ch 0316 008h]                Base Address : 0000000000000000
[144h 0324 008h]    Virtual GIC Base Address : 0000000000000000
[14Ch 0332 008h] Hypervisor GIC Base Address : 0000000000000000
[154h 0340 004h]       Virtual GIC Interrupt : 00000000
[158h 0344 008h]  Redistributor Base Address : 0000000000000000
[160h 0352 008h]                   ARM MPIDR : 0000000000000300
[168h 0360 001h]            Efficiency Class : 01
[169h 0361 001h]                    Reserved : 00
[16Ah 0362 002h]      SPE Overflow Interrupt : 0000
[16Ch 0364 002h]              TRBE Interrupt : 180C

[16Ch 0364 001h]               Subtable Type : 0C [Generic Interrupt Distributor]
[16Dh 0365 001h]                      Length : 18
[16Eh 0366 002h]                    Reserved : 0000
[170h 0368 004h]       Local GIC Hardware ID : 00000000
[174h 0372 008h]                Base Address : 0000000012000000
[17Ch 0380 004h]              Interrupt Base : 00000000
[180h 0384 001h]                     Version : 03
[181h 0385 003h]                    Reserved : 000000

[184h 0388 001h]               Subtable Type : 0E [Generic Interrupt Redistributor]
[185h 0389 001h]                      Length : 10
[187h 0391 002h]                    Reserved : 0000
[188h 0392 008h]                Base Address : 0000000012040000
[190h 0400 004h]                      Length : 00080000

Raw Table Data: Length 404 (0x194)

    0000: 41 50 49 43 94 01 00 00 05 6E 55 4E 49 53 4F 43  // APIC.....nUNISOC
    0010: 55 57 53 36 31 35 32 00 01 00 00 00 49 4E 54 4C  // UWS6152.....INTL
    0020: 04 04 25 20 00 00 00 00 00 00 00 00 0B 50 00 00  // ..% .........P..
    0030: 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00  // ................
    0040: 70 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // p...............
    0050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0070: 00 00 00 00 00 00 00 00 00 00 00 00 0B 50 00 00  // .............P..
    0080: 00 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00  // ................
    0090: 71 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // q...............
    00A0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    00B0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    00C0: 00 01 00 00 00 00 00 00 00 00 00 00 0B 50 00 00  // .............P..
    00D0: 00 00 00 00 02 00 00 00 01 00 00 00 00 00 00 00  // ................
    00E0: 72 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // r...............
    00F0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0100: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0110: 00 02 00 00 00 00 00 00 00 00 00 00 0B 50 00 00  // .............P..
    0120: 00 00 00 00 03 00 00 00 01 00 00 00 00 00 00 00  // ................
    0130: 73 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // s...............
    0140: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0150: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  // ................
    0160: 00 03 00 00 00 00 00 00 01 00 00 00 0C 18 00 00  // ................
    0170: 00 00 00 00 00 00 00 12 00 00 00 00 00 00 00 00  // ................
    0180: 03 00 00 00 0E 10 00 00 00 00 04 12 00 00 00 00  // ................
    0190: 00 00 08 00                                      // ....
