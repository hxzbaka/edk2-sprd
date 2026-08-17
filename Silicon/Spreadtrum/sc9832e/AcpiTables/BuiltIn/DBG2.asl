/*
 * SC9832E Debug Port Table 2.
 *
 * The live DT identifies UART1 as sprd,sc9836-uart at 0x70100000, size 0x100.
 * Windows debug support for this UART uses the custom SPRD kdcom binding,
 * represented by OEM-defined serial port type 0x8000 / subtype 0x0015.
 * Do not substitute the UMS512 table: it names URT0 and an unrelated USB port.
 */
[0004]                          Signature : "DBG2"
[0004]                       Table Length : 0000005C
[0001]                           Revision : 01
[0001]                           Checksum : 00
[0006]                             Oem ID : "UNISOC"
[0008]                       Oem Table ID : "SC9832E "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20230628

[0004]                         Info Offset : 0000002C
[0004]                          Info Count : 00000001

[0001]                           Revision : 01
[0002]                             Length : 0030
[0001]                     Register Count : 01
[0002]                    Namepath Length : 000A
[0002]                    Namepath Offset : 0026
[0002]                    OEM Data Length : 0000
[0002]                    OEM Data Offset : 0000
[0002]                         Port Type : 8000
[0002]                      Port Subtype : 0015
[0002]                          Reserved : 0000
[0002]               Base Address Offset : 0016
[0002]               Address Size Offset : 0022

[0012]             Base Address Register : [Generic Address Structure]
[0001]                           Space ID : 00
[0001]                          Bit Width : 20
[0001]                         Bit Offset : 00
[0001]              Encoded Access Width : 03
[0008]                            Address : 0000000070100000
[0004]                       Address Size : 00000100
[0010]                          Namepath : "\_SB.URT1"
