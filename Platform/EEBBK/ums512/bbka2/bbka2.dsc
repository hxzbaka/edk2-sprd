[Defines]
  VENDOR_NAME                    = EEBBK
  PLATFORM_NAME                  = bbka2
  PLATFORM_GUID                  = 28f1a3bf-193a-47e3-a7b9-5a435eaab2ee
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Spreadtrum/ums512/ums512.fdf
  DEVICE_DXE_FV_COMPONENTS       = Platform/EEBBK/ums512/bbka2/bbka2.fdf.inc

!include Platform/Spreadtrum/ums512/ums512.dsc

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DENABLE_SIMPLE_INIT -DLOW_RAM

[PcdsFixedAtBuild.common]
  gSprdTokenSpaceGuid.PcdMipiFrameBufferWidth|1200
  gSprdTokenSpaceGuid.PcdMipiFrameBufferHeight|1920

  gRenegadePkgTokenSpaceGuid.PcdDeviceVendor|"Unisoc"
  gRenegadePkgTokenSpaceGuid.PcdDeviceProduct|"bbka2"
  gRenegadePkgTokenSpaceGuid.PcdDeviceCodeName|"V1.1.0_221130"

  gSprdTokenSpaceGuid.PcdMipiDisplayInitialized|TRUE
