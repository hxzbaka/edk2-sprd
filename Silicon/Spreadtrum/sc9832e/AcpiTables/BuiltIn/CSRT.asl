/* SC9832E Core System Resource Table.
 * Live DT: ap-dma@20100000, sprd,ap-dma-v4.0, SPI50 -> GSIV82,
 * 28 channels, 32-bit DMA address width, maximum block 0x4000.
 */
[0004]                          Signature : "CSRT"
[0004]                       Table Length : 000001B4
[0001]                           Revision : 00
[0001]                           Checksum : 00
[0006]                             Oem ID : "UNISOC"
[0008]                       Oem Table ID : "SC9832E "
[0004]                       Oem Revision : 00000001
[0004]                    Asl Compiler ID : "INTL"
[0004]              Asl Compiler Revision : 20220531

/* One DMA resource group: 24-byte group header + 28-byte shared info +
 * controller descriptor + 28 channel descriptors = 400-byte resource group; 436 bytes including ACPI header.
 */
[0004]                             Length : 00000190
[0004]                          Vendor ID : 44525053
[0004]                       Subvendor ID : 00000000
[0002]                          Device ID : 0001
[0002]                       Subdevice ID : 0000
[0002]                           Revision : 0001
[0002]                           Reserved : 0000
[0004]                 Shared Info Length : 0000001C
[0002]                      Major Version : 0001
[0002]                      Minor Version : 0000
[0004]              MMIO Base Address Low : 20100000
[0004]             MMIO Base Address High : 00000000
[0004]                      GSI Interrupt : 00000052
[0001]                 Interrupt Polarity : 00
[0001]                     Interrupt Mode : 00
[0001]                       Num Channels : 1C
[0001]                  DMA Address Width : 20
[0002]                  Base Request Line : 0000
[0002]              Num Handshake Signals : 0000
[0004]                     Max Block Size : 00004000
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0001
[0004]                                UID : 414D5041
/* DMA channel 0. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 30304843
/* DMA channel 1. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 31304843
/* DMA channel 2. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 32304843
/* DMA channel 3. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 33304843
/* DMA channel 4. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 34304843
/* DMA channel 5. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 35304843
/* DMA channel 6. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 36304843
/* DMA channel 7. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 37304843
/* DMA channel 8. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 38304843
/* DMA channel 9. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 39304843
/* DMA channel 10. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 30314843
/* DMA channel 11. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 31314843
/* DMA channel 12. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 32314843
/* DMA channel 13. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 33314843
/* DMA channel 14. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 34314843
/* DMA channel 15. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 35314843
/* DMA channel 16. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 36314843
/* DMA channel 17. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 37314843
/* DMA channel 18. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 38314843
/* DMA channel 19. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 39314843
/* DMA channel 20. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 30324843
/* DMA channel 21. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 31324843
/* DMA channel 22. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 32324843
/* DMA channel 23. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 33324843
/* DMA channel 24. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 34324843
/* DMA channel 25. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 35324843
/* DMA channel 26. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 36324843
/* DMA channel 27. */
[0004]                             Length : 0000000C
[0002]                               Type : 0003
[0002]                            Subtype : 0000
[0004]                                UID : 37324843
