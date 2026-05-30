[Defines]
  VENDOR_NAME                    = SPRD
  PLATFORM_NAME                  = rt
  PLATFORM_GUID                  = 28f1a3bf-193a-47e3-a7b9-5a435eaab2ee
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Spreadtrum/sc9832e/sc9832e.fdf
  DEVICE_DXE_FV_COMPONENTS       = Platform/SPRD/sc9832e/rt/rt.fdf.inc

!include Platform/Spreadtrum/sc9832e/sc9832e.dsc

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DENABLE_SIMPLE_INIT -DLOW_RAM

[PcdsFixedAtBuild.common]
  gSprdTokenSpaceGuid.PcdMipiFrameBufferWidth|720
  gSprdTokenSpaceGuid.PcdMipiFrameBufferHeight|1560

  gRenegadePkgTokenSpaceGuid.PcdDeviceVendor|"SPRD"
  gRenegadePkgTokenSpaceGuid.PcdDeviceProduct|"RT"
  gRenegadePkgTokenSpaceGuid.PcdDeviceCodeName|"rt"
