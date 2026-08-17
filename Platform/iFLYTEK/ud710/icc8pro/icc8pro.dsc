[Defines]
  VENDOR_NAME                    = iFLYTEK
  PLATFORM_NAME                  = icc8pro
  PLATFORM_GUID                  = 28f1a3bf-193a-47e3-a7b9-5a435eaab2ee
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Spreadtrum/ud710/ud710.fdf
  DEVICE_DXE_FV_COMPONENTS       = Platform/iFLYTEK/ud710/icc8pro/icc8pro.fdf.inc

!include Platform/Spreadtrum/ud710/ud710.dsc

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DENABLE_SIMPLE_INIT -DLOW_RAM

[PcdsFixedAtBuild.common]
  gSprdTokenSpaceGuid.PcdMipiFrameBufferWidth|1200
  gSprdTokenSpaceGuid.PcdMipiFrameBufferHeight|1920

  gRenegadePkgTokenSpaceGuid.PcdDeviceVendor|"Unisoc"
  gRenegadePkgTokenSpaceGuid.PcdDeviceProduct|"icc8pro"
  gRenegadePkgTokenSpaceGuid.PcdDeviceCodeName|"HWC-C8-MB_V01.0B01"

  gSprdTokenSpaceGuid.PcdMipiDisplayInitialized|TRUE
