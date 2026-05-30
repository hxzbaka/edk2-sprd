## @file
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.
#  Copyright (c) 2022, Xilin Wu. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################

[Defines]
  SOC_PLATFORM            = SC9832E
  USE_PHYSICAL_TIMER      = TRUE

!include Silicon/Spreadtrum/SprdPkg/SprdCommonDsc.inc

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # Starting address
!if $(LOW_RAM) == TRUE
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x40000000         # Limit to 1GB Size here
  gSprdTokenSpaceGuid.PcdUefiMemPoolSize|0x10000000         # 256MB for low RAM (was 736MB)
!else
  gArmTokenSpaceGuid.PcdSystemMemorySize|0xFDFA0000         # Limit to 4GB Size here
  gSprdTokenSpaceGuid.PcdUefiMemPoolSize|0x2E000000         # Original 736MB
!endif

  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0xBFC40000     # CPU Vectors
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|26000000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x12001000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x12002000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x0

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000850
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0xBFC00000      # UEFI Stack
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|44

  gSprdTokenSpaceGuid.PcdUefiMemPoolBase|0x90000000         # DXE Heap base address
  # PcdUefiMemPoolSize set conditionally above
  gSprdTokenSpaceGuid.PcdMipiFrameBufferAddress|0xBEEF0000

  gArmPlatformTokenSpaceGuid.PcdCoreCount|4
  gArmPlatformTokenSpaceGuid.PcdClusterCount|1

  #
  # SimpleInit
  #
  gSimpleInitTokenSpaceGuid.PcdDeviceTreeStore|0x84150000
  gSimpleInitTokenSpaceGuid.PcdLoggerdUseConsole|FALSE

 #
 # Uart
 #
  gSprdTokenSpaceGuid.PcdDebugUartPortBase|0x70100000

[LibraryClasses.common]
  # Ported from SurfaceDuoPkg
  AslUpdateLib|Silicon/Spreadtrum/SprdPkg/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf

  PlatformMemoryMapLib|Silicon/Spreadtrum/sc9832e/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf
  PlatformPeiLib|Silicon/Spreadtrum/sc9832e/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|Silicon/Spreadtrum/sc9832e/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  SpdPlatformDevicesLib|Silicon/Spreadtrum/sc9832e/Library/SpdPlatformDevicesLib/SpdPlatformDevicesLib.inf
  SOCSmbiosInfoLib|Silicon/Spreadtrum/sc9832e/Library/SOCSmbiosInfoLib/SOCSmbiosInfoLib.inf

[Components.common]
!include Platform/Spreadtrum/sc9832e/dxe.dsc.inc  