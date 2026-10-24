/*
 * Serial Port Console Redirection Table (SPCR)
 * Platform  : SPRD UMS512 (sharkl5pro)
 * OEM ID    : SPRD
 * OEM Table : UMS512
 *
 * UART     : ARM PL011 @ 0x70100000
 * Baud     : 115200, 8N1, 无流控
 * 用途     : Windows on Arm (WoA) 串口调试
 *
 * 编译: iasl spcr_sprd.asl
 * 格式: [ByteLength]  FieldName : HexFieldValue
 */

[0004]                          Signature : "SPCR"
[0004]                       Table Length : 00000050
[0001]                           Revision : 02
[0001]                           Checksum : 00
[0006]                             Oem ID : "SPRD  "
[0008]                       Oem Table ID : "UMS512  "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

/* Interface Type: 0x03 = ARM PL011 UART */
[0001]                     Interface Type : 03
[0003]                           Reserved : 000000

/* GAS: System Memory, 8-bit, Byte access, addr=0x70100000 */
[0012]               Serial Port Register : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 08
[0001]                         Bit Offset : 00
[0001]               Encoded Access Width : 01
[0008]                            Address : 0000000070100000

/* Interrupt Type: 0x08 = Bit3 → GIC (GICv2/v3), 纯轮询 GSI=0 */
[0001]                     Interrupt Type : 08
[0001]                PCAT-compatible IRQ : 00
[0004]                          Interrupt : 00000000

/* Baud: 0x07=115200  Parity: 0=None  StopBits: 1  FlowCtrl: 0=None */
[0001]                          Baud Rate : 07
[0001]                             Parity : 00
[0001]                          Stop Bits : 01
[0001]                       Flow Control : 00

/* Terminal: 0x00=VT100 */
[0001]                      Terminal Type : 00
[0001]                           Reserved : 00

/* 非 PCI 设备 */
[0002]                      PCI Device ID : FFFF
[0002]                      PCI Vendor ID : FFFF
[0001]                            PCI Bus : 00
[0001]                         PCI Device : 00
[0001]                       PCI Function : 00
[0004]                          PCI Flags : 00000000
[0001]                        PCI Segment : 00
[0004]                           Reserved : 00000000
