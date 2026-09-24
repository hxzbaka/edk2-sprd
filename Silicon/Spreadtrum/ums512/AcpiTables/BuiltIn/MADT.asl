/*
 * Multiple APIC Description Table (MADT)
 * Platform  : SPRD UMS512 (sharkl5pro)
 * OEM ID    : SPRD
 * OEM Table : UMS512
 *
 * CPU :
 *   Cluster0: 6x Cortex-A55  MPIDR 0x000~0x500
 *   Cluster1: 2x Cortex-A75  MPIDR 0x600~0x700
 *
 * GICv3:
 *   GICD : 0x12000000  size 0x20000
 *   GICR : 0x12040000  stride 0x20000 x8 = total 0x100000
 *   GICv2m MSI : 0x12000000 (v2m@0, offset 0 from GICD)
 *
 * PMU SPI: DTS <0x00 0x70..0x77 0x04>
 *   GSIV = intnum + 32  →  144(0x90)..151(0x97)
 *
 */

[0004]                          Signature : "APIC"
[0004]                       Table Length : 000002EC
[0001]                           Revision : 05
[0001]                           Checksum : 00
[0006]                             Oem ID : "SPRD  "
[0008]                       Oem Table ID : "UMS512  "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

[0004]                 Local Apic Address : 00000000
[0004]              Flags (decoded below) : 00000000
                      PC-AT Compatibility : 0

/*
 * ================================================================
 * CPU0: Cortex-A55, Cluster0/Core0
 *   MPIDR=0x0000  GICR=0x12040000  PMU GSIV=144(0x90)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000000
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000090
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 0000000012040000
[0008]                          ARM MPIDR : 0000000000000000
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU1: Cortex-A55, Cluster0/Core1
 *   MPIDR=0x0100  GICR=0x12060000  PMU GSIV=145(0x91)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000001
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000091
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 0000000012060000
[0008]                          ARM MPIDR : 0000000000000100
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU2: Cortex-A55, Cluster0/Core2
 *   MPIDR=0x0200  GICR=0x12080000  PMU GSIV=146(0x92)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000002
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000092
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 0000000012080000
[0008]                          ARM MPIDR : 0000000000000200
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU3: Cortex-A55, Cluster0/Core3
 *   MPIDR=0x0300  GICR=0x120A0000  PMU GSIV=147(0x93)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000003
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000093
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 00000000120A0000
[0008]                          ARM MPIDR : 0000000000000300
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU4: Cortex-A55, Cluster0/Core4
 *   MPIDR=0x0400  GICR=0x120C0000  PMU GSIV=148(0x94)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000004
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000094
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 00000000120C0000
[0008]                          ARM MPIDR : 0000000000000400
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU5: Cortex-A55, Cluster0/Core5
 *   MPIDR=0x0500  GICR=0x120E0000  PMU GSIV=149(0x95)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000005
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000095
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 00000000120E0000
[0008]                          ARM MPIDR : 0000000000000500
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU6: Cortex-A75, Cluster1/Core0  ← 大核 EfficiencyClass=1
 *   MPIDR=0x0600  GICR=0x12100000  PMU GSIV=150(0x96)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000006
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000096
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 0000000012100000
[0008]                          ARM MPIDR : 0000000000000600
[0001]                   Efficiency Class : 01
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * CPU7: Cortex-A75, Cluster1/Core1  ← 大核 EfficiencyClass=1
 *   MPIDR=0x0700  GICR=0x12120000  PMU GSIV=151(0x97)
 * ================================================================
 */
[0001]                      Subtable Type : 0B
[0001]                             Length : 50
[0002]                           Reserved : 0000
[0004]               CPU Interface Number : 00000000
[0004]                      Processor UID : 00000007
[0004]              Flags (decoded below) : 00000001
                        Processor Enabled : 1
       Performance Interrupt Trigger Mode : 0
       Virtual GIC Interrupt Trigger Mode : 0
[0004]           Parking Protocol Version : 00000000
[0004]              Performance Interrupt : 00000097
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000000000000
[0008]           Virtual GIC Base Address : 0000000000000000
[0008]        Hypervisor GIC Base Address : 0000000000000000
[0004]              Virtual GIC Interrupt : 00000000
[0008]         Redistributor Base Address : 0000000012120000
[0008]                          ARM MPIDR : 0000000000000700
[0001]                   Efficiency Class : 01
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/*
 * ================================================================
 * GICD: GIC Distributor @ 0x12000000, GICv3
 * ================================================================
 */
[0001]                      Subtable Type : 0C
[0001]                             Length : 18
[0002]                           Reserved : 0000
[0004]              Local GIC Hardware ID : 00000000
[0008]                       Base Address : 0000000012000000
[0004]                     Interrupt Base : 00000000
[0001]                            Version : 03
[0003]                           Reserved : 000000

/*
 * ================================================================
 * GIC MSI Frame (GICv2m): v2m@0, offset 0 from GICD → 0x12000000
 * SPI Count/Base = 0 表示从 MSI_TYPER 寄存器自动读取
 * ================================================================
 */
[0001]                      Subtable Type : 0D
[0001]                             Length : 18
[0002]                           Reserved : 0000
[0004]                       MSI Frame ID : 00000000
[0008]                       Base Address : 0000000012000000
[0004]              Flags (decoded below) : 00000000
                               Select SPI : 0
[0002]                          SPI Count : 0000
[0002]                           SPI Base : 0000

/*
 * ================================================================
 * GICR: Redistributor 发现区域
 *   Base=0x12040000  Length=0x100000 (stride 0x20000 x 8 CPUs)
 * ================================================================
 */
[0001]                      Subtable Type : 0E
[0001]                             Length : 10
[0002]                           Reserved : 0000
[0008]                       Base Address : 0000000012040000
[0004]                             Length : 00100000
