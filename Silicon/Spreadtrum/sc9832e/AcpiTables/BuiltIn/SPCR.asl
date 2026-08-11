/*
 * SC9832E Serial Port Console Redirection Table.
 *
 * Hardware source: live DT and Linux sprd_serial driver.
 *   compatible  = "sprd,sc9836-uart"
 *   base/size   = 0x70100000 / 0x100
 *   interrupt   = GIC_SPI 3, level-high => ACPI GSI 35
 *   access      = 32-bit MMIO (TXD=0x0, RXD=0x4, LSR=0x8, STS1=0xc)
 *
 * The SC9836 UART is neither 16550 nor PL011/SBSA. Interface Type 0x15 is
 * the platform's custom UART type, matching the DBG2 OEM serial subtype 0x15
 * consumed by the SPRD kdcom transport.
 */
[0004]                          Signature : "SPCR"
[0004]                       Table Length : 00000050
[0001]                           Revision : 02
[0001]                           Checksum : 00
[0006]                             Oem ID : "UNISOC"
[0008]                       Oem Table ID : "SC9832E "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20230628

/* Custom SPRD SC9836 UART interface; matches DBG2 Port Subtype 0x0015. */
[0001]                     Interface Type : 15
[0003]                           Reserved : 000000

/* SystemMemory, 32-bit register accesses, UART1 base 0x70100000. */
[0012]               Serial Port Register : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 20
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 03
[0008]                            Address : 0000000070100000

/* GIC interrupt, GIC_SPI 3 translated to GSI 35. */
[0001]                     Interrupt Type : 08
[0001]                PCAT-compatible IRQ : 00
[0004]                          Interrupt : 00000023

/* 115200 baud, no parity, one stop bit, no flow control. */
[0001]                          Baud Rate : 07
[0001]                             Parity : 00
[0001]                          Stop Bits : 01
[0001]                       Flow Control : 00
[0001]                      Terminal Type : 00
[0001]                           Reserved : 00
[0002]                      PCI Device ID : FFFF
[0002]                      PCI Vendor ID : FFFF
[0001]                            PCI Bus : 00
[0001]                         PCI Device : 00
[0001]                       PCI Function : 00
[0004]                          PCI Flags : 00000000
[0001]                        PCI Segment : 00
[0004]                           Reserved : 00000000
