/*
 * Generic Timer Description Table (GTDT)
 * Platform  : UNISOC SC9832E (SharkLE)
 * OEM ID    : SPRD
 * OEM Table : UMS512
 *
 * Live recovery DTB: arm,armv8-timer, 26 MHz.
 * interrupts = <GIC_PPI 14 IRQ_TYPE_LEVEL_LOW>,
 *              <GIC_PPI 10 IRQ_TYPE_LEVEL_LOW>.
 * PPI GSIV = PPI + 16: non-secure physical = 30; virtual = 26.
 * ACPI timer flags 2 encode level / active-low.
 *
 * 编译: iasl gtdt_sprd.asl
 * 格式: [ByteLength]  FieldName : HexFieldValue
 */

[0004]                          Signature : "GTDT"
[0004]                       Table Length : 00000060
[0001]                           Revision : 03
[0001]                           Checksum : 00
[0006]                             Oem ID : "UNISOC"
[0008]                       Oem Table ID : "SC9832E "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

/* 系统计数器控制寄存器基址: UMS512 不暴露 → 全F */
[0008]              Counter Block Address : FFFFFFFFFFFFFFFF

[0004]                           Reserved : 00000000

/* Secure EL1 timer is not advertised by the live DT. */
[0004]               Secure EL1 Interrupt : 00000000
[0004]          EL1 Flags (decoded below) : 00000000
                             Trigger Mode : 0
                                 Polarity : 0
                                Always On : 0

/* Non-Secure EL1 physical timer: PPI14 -> GSIV30, level/active-low. */
[0004]           Non-Secure EL1 Interrupt : 0000001E
[0004]         NEL1 Flags (decoded below) : 00000002
                             Trigger Mode : 0
                                 Polarity : 1
                                Always On : 0

/* EL1 virtual timer: PPI10 -> GSIV26, level/active-low. */
[0004]            Virtual Timer Interrupt : 0000001A
[0004]           VT Flags (decoded below) : 00000002
                             Trigger Mode : 0
                                 Polarity : 1
                                Always On : 0

/* EL2 timer is not advertised by the live DT. */
[0004]           Non-Secure EL2 Interrupt : 00000000
[0004]         NEL2 Flags (decoded below) : 00000000
                             Trigger Mode : 0
                                 Polarity : 0
                                Always On : 0

/* 系统计数器只读视图基址: UMS512 不暴露 → 全F */
[0008]         Counter Read Block Address : FFFFFFFFFFFFFFFF

/* 无平台计时器块 (无 SBSA Watchdog) */
[0004]               Platform Timer Count : 00000000
[0004]              Platform Timer Offset : 00000000

/* Secure EL2 Timer (ACPI 6.3 新增): UMS512 不使用 */
[0004]             Virtual EL2 Timer GSIV : 00000000
[0004]            Virtual EL2 Timer Flags : 00000000
