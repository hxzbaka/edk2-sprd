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
  SOC_PLATFORM            = ums9620_t157
  USE_PHYSICAL_TIMER      = TRUE

!include Silicon/Spreadtrum/SprdPkg/SprdCommonDsc.inc

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # Starting address
  gArmTokenSpaceGuid.PcdSystemMemorySize|0xBDFA0000         # Limit to 4GB Size here

  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x9ff8c000
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|26000000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x12000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x12040000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000850
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x9FF90000      # UEFI Stack
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|44

  gSprdTokenSpaceGuid.PcdUefiMemPoolBase|0xB1000000         # DXE Heap base address
  gSprdTokenSpaceGuid.PcdUefiMemPoolSize|0x0A000000         # PHIT must fit inside Pool A p4 (0xC3100000-0xCE000000, 175MB) after firmware-reserved carveouts
  # PcdUefiMemPoolSize set conditionally above
  gSprdTokenSpaceGuid.PcdMipiFrameBufferAddress|0xB0000000

  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2

  #
  # USB Mass Storage
  #
   gSprdTokenSpaceGuid.UsbMassStorage|TRUE

  #
  # SimpleInit
  #
  gSimpleInitTokenSpaceGuid.PcdDeviceTreeStore|0x84150000
  gSimpleInitTokenSpaceGuid.PcdLoggerdUseConsole|FALSE

 #
 # Uart
 #
  gSprdTokenSpaceGuid.PcdDebugUartPortBase|0x20210000

  gSprdTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|16

[LibraryClasses.common]
  # Ported from SurfaceDuoPkg
  AslUpdateLib|Silicon/Spreadtrum/SprdPkg/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf

  PlatformMemoryMapLib|Silicon/Spreadtrum/ums9620_t157/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf
  PlatformPeiLib|Silicon/Spreadtrum/ums9620_t157/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|Silicon/Spreadtrum/ums9620_t157/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  SpdPlatformDevicesLib|Silicon/Spreadtrum/ums9620_t157/Library/SpdPlatformDevicesLib/SpdPlatformDevicesLib.inf
  SOCSmbiosInfoLib|Silicon/Spreadtrum/ums9620_t157/Library/SOCSmbiosInfoLib/SOCSmbiosInfoLib.inf

[Components.common]
!include Platform/Spreadtrum/ums9620_t157/dxe.dsc.inc  