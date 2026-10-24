[Defines]
  VENDOR_NAME                    = Coolpad
  PLATFORM_NAME                  = y60
  PLATFORM_GUID                  = 28f1a3bf-193a-47e3-a7b9-5a435eaab2ee
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Spreadtrum/ums9620_t157/ums9620_t157.fdf
  DEVICE_DXE_FV_COMPONENTS       = Platform/Coolpad/ums9620_t157/y60/y60.fdf.inc
  DEFINE USE_CUSTOM_FBDXE          = TRUE

!include Platform/Spreadtrum/ums9620_t157/ums9620_t157.dsc

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DENABLE_SIMPLE_INIT -DLOW_RAM

[PcdsFixedAtBuild.common]
  gSprdTokenSpaceGuid.PcdMipiFrameBufferWidth|320
  gSprdTokenSpaceGuid.PcdMipiFrameBufferHeight|480

  gRenegadePkgTokenSpaceGuid.PcdDeviceVendor|"Unisoc"
  gRenegadePkgTokenSpaceGuid.PcdDeviceProduct|"y60"
  gRenegadePkgTokenSpaceGuid.PcdDeviceCodeName|"CP23NV3"

  gSprdTokenSpaceGuid.PcdMipiDisplayInitialized|TRUE

[PcdsPatchableInModule.common]
  # GraphicsConsole text mode: auto-select max (320/8=40col, 480/19=25row)
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0
