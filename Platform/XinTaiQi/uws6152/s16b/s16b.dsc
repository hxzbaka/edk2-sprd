[Defines]
  VENDOR_NAME                    = XinTaiQi
  PLATFORM_NAME                  = s16b
  PLATFORM_GUID                  = b520c711-18a7-46b6-9dbb-610cf01712ae
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Spreadtrum/uws6152/uws6152.fdf
  DEVICE_DXE_FV_COMPONENTS       = Platform/XinTaiQi/uws6152/s16b/s16b.fdf.inc
  SPRD_DISPLAY_PANEL_LIB         = Platform/XinTaiQi/uws6152/s16b/Library/S16bDisplayPanelLib/S16bDisplayPanelLib.inf

!include Platform/Spreadtrum/uws6152/uws6152.dsc

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DENABLE_SIMPLE_INIT

[PcdsFixedAtBuild.common]
  gSprdTokenSpaceGuid.PcdMipiFrameBufferAddress|0x9EB0C000
  gSprdTokenSpaceGuid.PcdMipiFrameBufferWidth|412
  gSprdTokenSpaceGuid.PcdMipiFrameBufferHeight|502
  gSprdTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32
  gSprdTokenSpaceGuid.PcdMipiPixelClock|15360000
  gSprdTokenSpaceGuid.PcdDsiPhyBitClock|460000
  gSprdTokenSpaceGuid.PcdDsiEscapeClock|20000
  gSprdTokenSpaceGuid.PcdDsiLaneCount|1
  gSprdTokenSpaceGuid.PcdPanelResetGpio|50
  gSprdTokenSpaceGuid.PcdReconfigureDisplay|TRUE

  # Video burst mode
  gSprdTokenSpaceGuid.PcdDisplayCmdMode|TRUE
  gSprdTokenSpaceGuid.PcdDisplaySwitchToVideoMode|TRUE
  gSprdTokenSpaceGuid.PcdDisplayVideoModeType|2
  gSprdTokenSpaceGuid.PcdDisplayHSync|4
  gSprdTokenSpaceGuid.PcdDisplayHBackPorch|32
  gSprdTokenSpaceGuid.PcdDisplayHFrontPorch|32
  gSprdTokenSpaceGuid.PcdDisplayVSync|4
  gSprdTokenSpaceGuid.PcdDisplayVBackPorch|12
  gSprdTokenSpaceGuid.PcdDisplayVFrontPorch|18
  gSprdTokenSpaceGuid.PcdDisplayHFlip|FALSE
  gSprdTokenSpaceGuid.PcdDpuBaseAddress|0x20300000
  gSprdTokenSpaceGuid.PcdDpuCtrlReg|0x0004
  gSprdTokenSpaceGuid.PcdDpuIntClrReg|0x01E4
  gSprdTokenSpaceGuid.PcdDpuIntStsReg|0x01E8
  gSprdTokenSpaceGuid.PcdDpuIntDoneBit|0x01
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlReg|0x01F0
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTeEnableBit|0x0100
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTeExternalBit|0x0400
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTePolarityBit|0x0200
  gSprdTokenSpaceGuid.PcdDpuIntTeBit|0x02
  gSprdTokenSpaceGuid.PcdDpuIntEnReg|0x01E0

  gEfiMdeModulePkgTokenSpaceGuid.PcdSdMmcPciHostControllerMmioBase|0x71400000

  gRenegadePkgTokenSpaceGuid.PcdDeviceVendor|"XinTaiQi"
  gRenegadePkgTokenSpaceGuid.PcdDeviceProduct|"S16B"
  gRenegadePkgTokenSpaceGuid.PcdDeviceCodeName|"s16b"
