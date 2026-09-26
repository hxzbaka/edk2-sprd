# SoC DXE drivers
  INF Silicon/Spreadtrum/ums9620_517/Drivers/SprdClockDxe/SprdClockDxe.inf
  INF Silicon/Spreadtrum/ums9620_517/Drivers/SprdGpioDxe/SprdGpioDxe.inf
  INF Silicon/Spreadtrum/ums9620_517/Drivers/SprdEmmcDxe/SprdEmmcDxe.inf
  INF Platform/Coolpad/ums9620_517/y70/Drivers/SprdButtonDxe/SprdButtonDxe.inf
  INF Silicon/Spreadtrum/ums9620_517/Drivers/SprdMusbDxe/SprdMusbDxe.inf
  # Produces gSprdDisplayReadyProtocolGuid, required by SimpleFbDxe.
  INF Silicon/Spreadtrum/ums9620_517/Drivers/SprdDisplayDxe/SprdDisplayDxe.inf

  # SD card support (SdDxe for SDHCI, DiskIo/Partition/FAT already in SprdCommonDsc.inc)
  INF MdeModulePkg/Bus/Sd/SdDxe/SdDxe.inf
