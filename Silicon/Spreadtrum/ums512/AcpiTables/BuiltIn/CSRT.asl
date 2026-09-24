/*
 * Core System Resource Table (CSRT)
 * Platform  : SPRD UMS512 (sharkl5pro)
 * OEM ID    : SPRD
 * OEM Table : UMS512
 *
 * DTS:
 *
 *   DMA1 @ 0x20000000  (AP-AHB, sharkl5pro-dma)
 *     interrupts = <0x00 0x18 0x04>  SPI 24 → GSIV 56 (0x38)
 *     #dma-channels = <0x1c>  (28 channels)
 *     DMA Address Width = 32-bit (0x20)
 *     Max Block Size = 0x4000
 *
 *   DMA2 @ 0x33580000  (sharkl5pro-dma)
 *     interrupts = <0x00 0xb4 0x04>  SPI 180 → GSIV 212 (0xD4)
 *     #dma-channels = <0x20>  (32 channels)
 *     DMA Address Width = 32-bit (0x20)
 *     Max Block Size = 0x4000
 *
 * Resource Group :
 *   RG1 (DMA1): ResourceGroup Header(24) + SharedInfo(28) +
 *               1 Controller(APMA) + 6 Channels(CHA0~CHA5) = 7 resources
 *               Length = 24+28+7*12 = 136 = 0x88
 *
 *   RG2 (DMA2): ResourceGroup Header(24) + SharedInfo(28) +
 *               1 Controller(GAMA) + 8 Channels(CHA0~CHA7) = 9 resources
 *               Length = 24+28+9*12 = 160 = 0xA0
 *
 * Table Total = 36(ACPI header) + 136 + 160 = 332 = 0x14C
 *
 * Vendor ID: 'SPRD' stored LE = 0x44525053
 *
 */

[0004]                          Signature : "CSRT"
[0004]                       Table Length : 0000014C
[0001]                           Revision : 00
[0001]                           Checksum : 00
[0006]                             Oem ID : "SPRD  "
[0008]                       Oem Table ID : "UMS512  "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

/*
 * ================================================================
 * Resource Group 1: DMA1 @ 0x20000000
 *   sprd,sharkl5pro-dma, 28 channels, SPI 24 (GSIV=56=0x38)
 * ================================================================
 */
[0004]                             Length : 00000088
[0004]                          Vendor ID : 44525053
[0004]                       Subvendor ID : 00000000
[0002]                          Device ID : 0001
[0002]                       Subdevice ID : 0000
[0002]                           Revision : 0001
[0002]                           Reserved : 0000
[0004]                 Shared Info Length : 0000001C

/* Shared Info (28 bytes) */
[0002]                      Major Version : 0001
[0002]                      Minor Version : 0000
[0004]              MMIO Base Address Low : 20000000
[0004]             MMIO Base Address High : 00000000
[0004]                      GSI Interrupt : 00000038
[0001]                 Interrupt Polarity : 00
[0001]                     Interrupt Mode : 00
[0001]                       Num Channels : 1C
[0001]                  DMA Address Width : 20
[0002]                  Base Request Line : 0000
[0002]              Num Handshake Signals : 0000
[0004]                     Max Block Size : 00004000

/* Resource Descriptor: DMA Controller (Type=0003 Subtype=0001) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0001
[0004]                                UID : 41504D41

/* Resource Descriptor: DMA Channel 0 (CHA0) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 30414843

/* Resource Descriptor: DMA Channel 1 (CHA1) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 31414843

/* Resource Descriptor: DMA Channel 2 (CHA2) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 32414843

/* Resource Descriptor: DMA Channel 3 (CHA3) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 33414843

/* Resource Descriptor: DMA Channel 4 (CHA4) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 34414843

/* Resource Descriptor: DMA Channel 5 (CHA5) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 35414843

/*
 * ================================================================
 * Resource Group 2: DMA2 @ 0x33580000
 *   sprd,sharkl5pro-dma, 32 channels, SPI 180 (GSIV=212=0xD4)
 * ================================================================
 */
[0004]                             Length : 000000A0
[0004]                          Vendor ID : 44525053
[0004]                       Subvendor ID : 00000000
[0002]                          Device ID : 0002
[0002]                       Subdevice ID : 0000
[0002]                           Revision : 0001
[0002]                           Reserved : 0000
[0004]                 Shared Info Length : 0000001C

/* Shared Info (28 bytes) */
[0002]                      Major Version : 0001
[0002]                      Minor Version : 0000
[0004]              MMIO Base Address Low : 33580000
[0004]             MMIO Base Address High : 00000000
[0004]                      GSI Interrupt : 000000D4
[0001]                 Interrupt Polarity : 00
[0001]                     Interrupt Mode : 00
[0001]                       Num Channels : 20
[0001]                  DMA Address Width : 20
[0002]                  Base Request Line : 0000
[0002]              Num Handshake Signals : 0000
[0004]                     Max Block Size : 00004000

/* Resource Descriptor: DMA Controller (Type=0003 Subtype=0001) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0001
[0004]                                UID : 47414D41

/* Resource Descriptor: DMA Channel 0 (CHA0) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 30414843

/* Resource Descriptor: DMA Channel 1 (CHA1) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 31414843

/* Resource Descriptor: DMA Channel 2 (CHA2) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 32414843

/* Resource Descriptor: DMA Channel 3 (CHA3) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 33414843

/* Resource Descriptor: DMA Channel 4 (CHA4) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 34414843

/* Resource Descriptor: DMA Channel 5 (CHA5) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 35414843

/* Resource Descriptor: DMA Channel 6 (CHA6) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 36414843

/* Resource Descriptor: DMA Channel 7 (CHA7) */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 37414843
