## @file
#  UWS6152 (SharkL5) Platform DSC
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
##

[Defines]
  SOC_PLATFORM            = uws6152
  USE_PHYSICAL_TIMER      = TRUE
  USE_CUSTOM_FBDXE        = TRUE
  USE_CUSTOM_CONSOLE      = TRUE

!include Silicon/Spreadtrum/SprdPkg/SprdCommonDsc.inc

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  # 4GB - 4KB
  gArmTokenSpaceGuid.PcdSystemMemorySize|0xFFFFF000

  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0xBFFC0000
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|26000000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x12000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x12040000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000615
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0xBD000000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|44

  gSprdTokenSpaceGuid.PcdUefiMemPoolBase|0xA1800000
  gSprdTokenSpaceGuid.PcdUefiMemPoolSize|0x1B800000
  gSprdTokenSpaceGuid.PcdMipiFrameBufferAddress|0x9EB0C000

  gArmPlatformTokenSpaceGuid.PcdCoreCount|4
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2

  #
  # SimpleInit
  #
  gSimpleInitTokenSpaceGuid.PcdDeviceTreeStore|0x84150000
  gSimpleInitTokenSpaceGuid.PcdLoggerdUseConsole|FALSE

  #
  # Uart
  #
  gSprdTokenSpaceGuid.PcdDebugUartPortBase|0x70100000

  #
  # Display
  #
  gSprdTokenSpaceGuid.PcdDisplayCmdMode|FALSE
  gSprdTokenSpaceGuid.PcdDisplayHFlip|FALSE
  gSprdTokenSpaceGuid.PcdApAhbBaseAddress|0x20100000
  gSprdTokenSpaceGuid.PcdApClkBaseAddress|0x20200000
  gSprdTokenSpaceGuid.PcdPmuApbBaseAddress|0x327E0000
  gSprdTokenSpaceGuid.PcdDpuBaseAddress|0x20300000
  gSprdTokenSpaceGuid.PcdDsiBaseAddress|0x20400000
  gSprdTokenSpaceGuid.PcdApGpioBaseAddress|0x32070000
  gSprdTokenSpaceGuid.PcdDpuCtrlReg|0x0004
  gSprdTokenSpaceGuid.PcdDpuIntClrReg|0x01E4
  gSprdTokenSpaceGuid.PcdDpuIntStsReg|0x01E8
  gSprdTokenSpaceGuid.PcdDpuIntDoneBit|0x01
  gSprdTokenSpaceGuid.PcdReconfigureDisplay|FALSE
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlReg|0x01F0
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTeEnableBit|0x0100
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTeExternalBit|0x0400
  gSprdTokenSpaceGuid.PcdDpuDpiCtrlTePolarityBit|0x0200
  gSprdTokenSpaceGuid.PcdDpuIntTeBit|0x02
  gSprdTokenSpaceGuid.PcdDpuIntEnReg|0x01E0
  gSprdTokenSpaceGuid.PcdDpuInterruptSource|59

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0

[LibraryClasses.common]
  # Ported from SurfaceDuoPkg
  AslUpdateLib|Silicon/Spreadtrum/SprdPkg/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf

  PlatformMemoryMapLib|Silicon/Spreadtrum/uws6152/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf
  PlatformPeiLib|Silicon/Spreadtrum/uws6152/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|Silicon/Spreadtrum/uws6152/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  SpdPlatformDevicesLib|Silicon/Spreadtrum/uws6152/Library/SpdPlatformDevicesLib/SpdPlatformDevicesLib.inf
  SOCSmbiosInfoLib|Silicon/Spreadtrum/uws6152/Library/SOCSmbiosInfoLib/SOCSmbiosInfoLib.inf

[Components.common]
!include Platform/Spreadtrum/uws6152/dxe.dsc.inc
