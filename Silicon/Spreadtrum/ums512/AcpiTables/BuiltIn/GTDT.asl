/*
 * Generic Timer Description Table (GTDT)
 * Platform  : SPRD UMS512 (sharkl5pro)
 * OEM ID    : SPRD
 * OEM Table : UMS512
 *
 * DTS :
 *   timer { compatible = "arm,armv8-timer";
 *     interrupts = <0x01 0x0d 0x04   PPI13 → Secure EL1 Physical
 *                   0x01 0x0e 0x04   PPI14 → Non-Secure EL1 Physical
 *                   0x01 0x0b 0x04   PPI11 → EL1 Virtual
 *                   0x01 0x0a 0x04>; PPI10 → EL2 Hypervisor
 *     arm,no-tick-in-suspend; }
 *
 * PPI GSIV = intnum + 16:
 *   PPI13 → GSIV 29(0x1D)   PPI14 → GSIV 30(0x1E)
 *   PPI11 → GSIV 27(0x1B)   PPI10 → GSIV 26(0x1A)
 *
 * Timer Flags (decoded below):
 *   Trigger Mode : 0 = Level (DTS IRQ_TYPE_LEVEL_HIGH=0x04)
 *   Polarity     : 0 = Active-High
 *   Always On    : 0 (arm,no-tick-in-suspend)
 *
 */

[0004]                          Signature : "GTDT"
[0004]                       Table Length : 00000060
[0001]                           Revision : 03
[0001]                           Checksum : 00
[0006]                             Oem ID : "SPRD  "
[0008]                       Oem Table ID : "UMS512  "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

[0008]              Counter Block Address : FFFFFFFFFFFFFFFF

[0004]                           Reserved : 00000000

/* Secure EL1 Physical Timer: PPI13, GSIV=29=0x1D, Level-High */
[0004]               Secure EL1 Interrupt : 0000001D
[0004]          EL1 Flags (decoded below) : 00000000
                             Trigger Mode : 0
                                 Polarity : 0
                                Always On : 0

/* Non-Secure EL1 Physical Timer: PPI14, GSIV=30=0x1E, Level-High */
[0004]           Non-Secure EL1 Interrupt : 0000001E
[0004]         NEL1 Flags (decoded below) : 00000000
                             Trigger Mode : 0
                                 Polarity : 0
                                Always On : 0

/* EL1 Virtual Timer: PPI11, GSIV=27=0x1B, Level-High */
[0004]            Virtual Timer Interrupt : 0000001B
[0004]           VT Flags (decoded below) : 00000000
                             Trigger Mode : 0
                                 Polarity : 0
                                Always On : 0

/* EL2 Non-Secure (Hypervisor) Timer: PPI10, GSIV=26=0x1A, Level-High */
[0004]           Non-Secure EL2 Interrupt : 0000001A
[0004]         NEL2 Flags (decoded below) : 00000000
                             Trigger Mode : 0
                                 Polarity : 0
                                Always On : 0

[0008]         Counter Read Block Address : FFFFFFFFFFFFFFFF

[0004]               Platform Timer Count : 00000000
[0004]              Platform Timer Offset : 00000000

[0004]             Virtual EL2 Timer GSIV : 00000000
[0004]            Virtual EL2 Timer Flags : 00000000
