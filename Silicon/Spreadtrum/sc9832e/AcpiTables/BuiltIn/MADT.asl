/* SC9832E (SharkLE) MADT. Source: live recovery DTB.
 * GIC-400/v2: GICD 12001000, GICC 12002000, GICH 12004000, GICV 12006000.
 * Four Cortex-A53 CPUs, MPIDR 0..3. GIC maintenance PPI9 -> GSIV 25.
 * PMU has SPI92..95 -> GSIV 124..127, one per CPU.
 */
[0004]                          Signature : "APIC"
[0004]                       Table Length : 00000184
[0001]                           Revision : 05
[0001]                           Checksum : 00
[0006]                             Oem ID : "UNISOC"
[0008]                       Oem Table ID : "SC9832E "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

[0004]                 Local Apic Address : 00000000
[0004]              Flags (decoded below) : 00000000
                      PC-AT Compatibility : 0

/* CPU0: Cortex-A53, MPIDR 0, PMU SPI92 -> GSIV 124. */
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
[0004]              Performance Interrupt : 0000007C
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000012002000
[0008]           Virtual GIC Base Address : 0000000012006000
[0008]        Hypervisor GIC Base Address : 0000000012004000
[0004]              Virtual GIC Interrupt : 00000019
[0008]         Redistributor Base Address : 0000000000000000
[0008]                          ARM MPIDR : 0000000000000000
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/* CPU1: Cortex-A53, MPIDR 1, PMU SPI93 -> GSIV 125. */
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
[0004]              Performance Interrupt : 0000007D
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000012002000
[0008]           Virtual GIC Base Address : 0000000012006000
[0008]        Hypervisor GIC Base Address : 0000000012004000
[0004]              Virtual GIC Interrupt : 00000019
[0008]         Redistributor Base Address : 0000000000000000
[0008]                          ARM MPIDR : 0000000000000001
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/* CPU2: Cortex-A53, MPIDR 2, PMU SPI94 -> GSIV 126. */
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
[0004]              Performance Interrupt : 0000007E
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000012002000
[0008]           Virtual GIC Base Address : 0000000012006000
[0008]        Hypervisor GIC Base Address : 0000000012004000
[0004]              Virtual GIC Interrupt : 00000019
[0008]         Redistributor Base Address : 0000000000000000
[0008]                          ARM MPIDR : 0000000000000002
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/* CPU3: Cortex-A53, MPIDR 3, PMU SPI95 -> GSIV 127. */
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
[0004]              Performance Interrupt : 0000007F
[0008]                     Parked Address : 0000000000000000
[0008]                       Base Address : 0000000012002000
[0008]           Virtual GIC Base Address : 0000000012006000
[0008]        Hypervisor GIC Base Address : 0000000012004000
[0004]              Virtual GIC Interrupt : 00000019
[0008]         Redistributor Base Address : 0000000000000000
[0008]                          ARM MPIDR : 0000000000000003
[0001]                   Efficiency Class : 00
[0001]                           Reserved : 00
[0002]             SPE Overflow Interrupt : 0000
[0002]                     TRBE Interrupt : 0000

/* GIC Distributor, GICv2. */
[0001]                      Subtable Type : 0C
[0001]                             Length : 18
[0002]                           Reserved : 0000
[0004]              Local GIC Hardware ID : 00000000
[0008]                       Base Address : 0000000012001000
[0004]                     Interrupt Base : 00000000
[0001]                            Version : 02
[0003]                           Reserved : 000000
