[Defines]
  VENDOR_NAME                    = divo
  PLATFORM_NAME                  = dw99
  PLATFORM_GUID                  = 6D4FCE02-B2BB-4A4D-905D-A0659540ACCD
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  USE_CUSTOM_CONSOLE             = TRUE
  DEVICE_SIMPLE_FB_DRIVER        = Platform/divo/sc9832e/dw99/Drivers/SimpleFbDxe/SimpleFbDxe.inf
  FLASH_DEFINITION               = Platform/Spreadtrum/sc9832e/sc9832e.fdf
  DEVICE_DXE_FV_COMPONENTS       = Platform/divo/sc9832e/dw99/dw99.fdf.inc

!include Platform/Spreadtrum/sc9832e/sc9832e.dsc

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DENABLE_SIMPLE_INIT

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdSystemMemorySize|0xC0000000
  gSprdTokenSpaceGuid.PcdUefiMemPoolBase|0xC0000000
  gSprdTokenSpaceGuid.PcdUefiMemPoolSize|0x2DEF0000
  gSimpleInitTokenSpaceGuid.PcdDeviceTreeStore|0xBFEF0000

  gSprdTokenSpaceGuid.PcdMipiFrameBufferWidth|368
  gSprdTokenSpaceGuid.PcdMipiFrameBufferHeight|448
  gSprdTokenSpaceGuid.PcdMipiFrameBufferVisibleWidth|368
  gSprdTokenSpaceGuid.PcdMipiFrameBufferVisibleHeight|448
  gSprdTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32
  gSprdTokenSpaceGuid.PcdDisplayCmdMode|TRUE
  gSprdTokenSpaceGuid.PcdDisplaySwitchToVideoMode|FALSE
  gSprdTokenSpaceGuid.PcdDisplayVideoModeType|2
  gSprdTokenSpaceGuid.PcdDisplayHSync|2
  gSprdTokenSpaceGuid.PcdDisplayHBackPorch|8
  gSprdTokenSpaceGuid.PcdDisplayHFrontPorch|10
  gSprdTokenSpaceGuid.PcdDisplayVSync|2
  gSprdTokenSpaceGuid.PcdDisplayVBackPorch|12
  gSprdTokenSpaceGuid.PcdDisplayVFrontPorch|16
  gSprdTokenSpaceGuid.PcdMipiPixelClock|10667000
  gSprdTokenSpaceGuid.PcdDsiPhyBitClock|306000
  gSprdTokenSpaceGuid.PcdDsiEscapeClock|20000
  gSprdTokenSpaceGuid.PcdDsiLaneCount|1
  gSprdTokenSpaceGuid.PcdPanelResetGpio|50
  gSprdTokenSpaceGuid.PcdPanelAvddGpio|33
  gSprdTokenSpaceGuid.PcdApGpioBaseAddress|0x40280000
  gSprdTokenSpaceGuid.PcdDpuBaseAddress|0x20800000
  gSprdTokenSpaceGuid.PcdDpuIntClrReg|0x0164
  gSprdTokenSpaceGuid.PcdDpuIntStsReg|0x0168
  gSprdTokenSpaceGuid.PcdDpuIntEnReg|0x0160
  gSprdTokenSpaceGuid.PcdDpuIntDoneBit|0x01
  gSprdTokenSpaceGuid.PcdDpuIntTeBit|0x02
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlReg|0x0170
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTeEnableBit|0x0100
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTeExternalBit|0x0400
  gSprdTokenSpaceGuid.PcdDpuInterruptSource|78
  gSprdTokenSpaceGuid.PcdDsiBaseAddress|0x21800000
  gSprdTokenSpaceGuid.PcdApAhbBaseAddress|0x20E00000
  gSprdTokenSpaceGuid.PcdApClkBaseAddress|0x21500000
  gSprdTokenSpaceGuid.PcdPmuApbBaseAddress|0x402B0000
  gSprdTokenSpaceGuid.PcdReconfigureDisplay|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdSdMmcPciHostControllerMmioBase|0x20600000

  gRenegadePkgTokenSpaceGuid.PcdDeviceVendor|"divo"
  gRenegadePkgTokenSpaceGuid.PcdDeviceProduct|"DW99"
  gRenegadePkgTokenSpaceGuid.PcdDeviceCodeName|"dw99"

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0

[LibraryClasses.common]
!if $(USE_UART) != 1
  SerialPortLib|Platform/divo/sc9832e/dw99/Library/FrameBufferSerialPortLib/FrameBufferSerialPortLib.inf
!endif
  PlatformMemoryMapLib|Platform/divo/sc9832e/dw99/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf
  SOCSmbiosInfoLib|Platform/divo/sc9832e/dw99/Library/SOCSmbiosInfoLib/SOCSmbiosInfoLib.inf
  SprdDisplayPanelLib|Platform/divo/sc9832e/dw99/Library/Dw99DisplayPanelLib/Dw99DisplayPanelLib.inf

[Components.common]
  Platform/divo/sc9832e/dw99/Drivers/SimpleFbDxe/SimpleFbDxe.inf
  Platform/divo/sc9832e/dw99/Drivers/SprdDisplayDxe/SprdDisplayDxe.inf
  Silicon/Spreadtrum/uws6152/Drivers/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

!if $(USE_UART) != 1
[Components.AARCH64]
  Silicon/Spreadtrum/SprdPkg/PrePi/PrePi.inf {
    <LibraryClasses>
      SerialPortLib|Platform/divo/sc9832e/dw99/Library/FrameBufferSerialPortLib/FrameBufferSerialPortLib.inf
  }
!if $(NO_EXCEPTION_DISPLAY) != TRUE
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf {
    <LibraryClasses>
      SerialPortLib|Platform/divo/sc9832e/dw99/Library/FrameBufferSerialPortLib/FrameBufferSerialPortLib.inf
  }
!endif
!endif
