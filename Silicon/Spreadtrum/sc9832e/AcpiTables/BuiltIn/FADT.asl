/*
 * Fixed ACPI Description Table (FADT/FACP)
 * Platform  : UNISOC SC9832E (SharkLE)
 * OEM ID    : SPRD
 * OEM Table : UMS512
 *
 * 策略: Hardware-Reduced ACPI + Low Power S0 Idle
 *   - 完全放弃传统 PC 寄存器 (PM1A/PM1B/PM2/PMTimer/GPE)
 *   - 电源管理依赖 PSCI (SMC 调用) + ACPI 硬件简化模式
 *   - 使能 Low Power S0 Idle (Modern Standby) 支持
 *   - ARM Flags: PSCI Compliant=1, Use HVC=0 (用 SMC)
 *
 * ACPI Spec 6.x, Section 5.2.9
 * 编译: iasl fadt_sprd.asl
 */

[0004]                          Signature : "FACP"
[0004]                       Table Length : 00000114
[0001]                           Revision : 06
[0001]                           Checksum : 00
[0006]                             Oem ID : "UNISOC"
[0008]                       Oem Table ID : "SC9832E "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

/*
 * 传统 32-bit 指针: Hardware-Reduced 模式下全部为 0
 * FACS/DSDT 使用下方 64-bit X_FACS / X_DSDT 字段
 */
[0004]                       FACS Address : 00000000
[0004]                       DSDT Address : 00000000

[0001]                              Model : 00
[0001]                         PM Profile : 02        /* Mobile */
[0002]                      SCI Interrupt : 0000      /* HW-Reduced: 无 SCI IRQ */
[0004]                   SMI Command Port : 00000000  /* 无 SMI */
[0001]                  ACPI Enable Value : 00
[0001]                 ACPI Disable Value : 00
[0001]                     S4BIOS Command : 00
[0001]                    P-State Control : 00

/* 传统 PM 寄存器块: HW-Reduced 全为 0 */
[0004]           PM1A Event Block Address : 00000000
[0004]           PM1B Event Block Address : 00000000
[0004]         PM1A Control Block Address : 00000000
[0004]         PM1B Control Block Address : 00000000
[0004]          PM2 Control Block Address : 00000000
[0004]             PM Timer Block Address : 00000000
[0004]                 GPE0 Block Address : 00000000
[0004]                 GPE1 Block Address : 00000000

/* 寄存器块长度: HW-Reduced 全为 0 */
[0001]             PM1 Event Block Length : 00
[0001]           PM1 Control Block Length : 00
[0001]           PM2 Control Block Length : 00
[0001]              PM Timer Block Length : 00
[0001]                  GPE0 Block Length : 00
[0001]                  GPE1 Block Length : 00
[0001]                   GPE1 Base Offset : 00

[0001]                       _CST Support : 00
[0002]                         C2 Latency : 0000
[0002]                         C3 Latency : 0000
[0002]                     CPU Cache Size : 0000
[0002]                 Cache Flush Stride : 0000
[0001]                  Duty Cycle Offset : 00
[0001]                   Duty Cycle Width : 00
[0001]                RTC Day Alarm Index : 00
[0001]              RTC Month Alarm Index : 00
[0001]                  RTC Century Index : 00

/*
 * IA-PC Boot Flags: 移动平台简化标志
 *   Bit2 (VGA Not Present)     = 1  → 无传统 VGA
 *   Bit3 (MSI Not Supported)   = 0
 *   Bit5 (CMOS RTC Not Present)= 1  → 无 CMOS RTC
 */
[0002]         Boot Flags (decoded below) : 0024
            Legacy Devices Supported (V2) : 0
         8042 Present on ports 60/64 (V2) : 0
                     VGA Not Present (V4) : 1
                   MSI Not Supported (V4) : 0
             PCIe ASPM Not Supported (V4) : 0
                CMOS RTC Not Present (V5) : 1

[0001]                           Reserved : 00

/*
 * FADT Flags:
 *   Bit4  (CTRL_METHOD_PWR_BTN) = 1  → 电源键通过控制方法
 *   Bit5  (CTRL_METHOD_SLP_BTN) = 1  → 睡眠键通过控制方法
 *   Bit10 (RESET_REG_SUP)       = 0  → HW-Reduced 无需 Reset Reg
 *   Bit20 (HW_REDUCED_ACPI)     = 1  → 硬件简化 ACPI ★
 *   Bit21 (LOW_POWER_S0_IDLE)   = 1  → Modern Standby ★
 *
 * Bits [20:21] = 0b11 → 0x300000
 * Bits [4:5]   = 0b11 → 0x000030
 * Combined     = 0x300030
 */
[0004]              Flags (decoded below) : 00300030
   WBINVD instruction is operational (V1) : 0
           WBINVD flushes all caches (V1) : 0
                 All CPUs support C1 (V1) : 0
               C2 works on MP system (V1) : 0
         Control Method Power Button (V1) : 1
         Control Method Sleep Button (V1) : 1
     RTC wake not in fixed reg space (V1) : 0
         RTC can wake system from S4 (V1) : 0
                     32-bit PM Timer (V1) : 0
                   Docking Supported (V1) : 0
            Reset Register Supported (V2) : 0
                         Sealed Case (V3) : 0
                 Headless - No Video (V3) : 0
     Use native instr after SLP_TYPx (V3) : 0
           PCIEXP_WAK Bits Supported (V4) : 0
                  Use Platform Timer (V4) : 0
            RTC_STS valid on S4 wake (V4) : 0
             Remote Power-on capable (V4) : 0
              Use APIC Cluster Model (V4) : 0
  Use APIC Physical Destination Mode (V4) : 0
                    Hardware Reduced (V5) : 1
                   Low Power S0 Idle (V5) : 1

/*
 * Reset Register: HW-Reduced 模式下不需要, 填全0 GAS
 */
[0012]                     Reset Register : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0001]               Value to cause reset : 00

/*
 * ARM Flags:
 *   Bit0 (PSCI Compliant) = 1  → 使用 PSCI 做 CPU 控制/电源
 *   Bit1 (Must use HVC)   = 0  → 使用 SMC (不是 HVC)
 */
[0002]          ARM Flags (decoded below) : 0001
                           PSCI Compliant : 1
                    Must use HVC for PSCI : 0

[0001]                FADT Minor Revision : 00

/* 64-bit 扩展指针: FACS 不需要 (HW-Reduced), DSDT 指向实际 DSDT 表 */
[0008]                       FACS Address : 0000000000000000
[0008]                       DSDT Address : 0000000000000001  /* 占位符, 固件安装时由UEFI填入实际地址 */

/* 扩展 PM 寄存器 GAS: HW-Reduced 全部填空 GAS */
[0012]                   PM1A Event Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                   PM1B Event Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                 PM1A Control Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                 PM1B Control Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                  PM2 Control Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                     PM Timer Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                         GPE0 Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]                         GPE1 Block : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]             Sleep Control Register : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0012]              Sleep Status Register : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 00
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 00
[0008]                            Address : 0000000000000000

[0008]                      Hypervisor ID : 0000000000000000
