/** @file
  Spreadtrum UWS6152 Platform Devices Library.

  Provides chip information, ACPI table patches, and CPU frequency control (stubbed).

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/AslUpdateLib.h>
#include <Library/MemoryMapHelperLib.h>

#define SPRD_CHIP_ID          0x6152
#define SPRD_CHIP_VERSION     0x01
#define SPRD_CHIP_NAME        "UWS6152"
#define SPRD_PLATFORM_SUBTYPE 0

// ADI
#define EFUSE_BASE           0x300
#define UID_LOW_OFFSET        0x5E
#define UID_HIGH_OFFSET       0x5F

//
//STATIC
//UINT64
//GetSprdUid(VOID)
//{
//    UINT32 Low, High;
//
    // 注意：eFuse 可能需要使能时钟，此处假设已经被其他驱动初始化
//    Low  = MmioRead32(EFUSE_BASE + UID_LOW_OFFSET * 4);
//    High = MmioRead32(EFUSE_BASE + UID_HIGH_OFFSET * 4);

//    return ((UINT64)High << 32) | Low;
//       return 1;
//}
//

STATIC
EFI_STATUS
SetupSprdCpuPerfLevels(VOID)
{
    DEBUG((EFI_D_INFO, "SetupSprdCpuPerfLevels: Stub, CPU frequency not changed.\n"));
    // TODO: 从内核驱动（sprd-cpufreq）提取寄存器操作实现真正的频率提升
    return EFI_SUCCESS;
}

//
//STATIC
//VOID
//UpdateAcpiTablesForSprd(VOID)
//{
//    UINT32 ChipId, ChipVer, PlatformSubtype;
//    CHAR8  ChipName[16];
//    UINT64 Uid;
//
//    ChipId = SPRD_CHIP_ID;
//    ChipVer = SPRD_CHIP_VERSION;
//    AsciiStrCpyS(ChipName, sizeof(ChipName), SPRD_CHIP_NAME);
//    PlatformSubtype = SPRD_PLATFORM_SUBTYPE;
//    Uid = GetSprdUid();
//
//    DEBUG((EFI_D_INFO, "Platform: Chip ID=0x%x, Version=0x%x, Name=%a, Subtype=%d, UID=0x%llx\n",
//           ChipId, ChipVer, ChipName, PlatformSubtype, Uid));
//
//    UpdateNameAslCode(SIGNATURE_32('S', 'O', 'I', 'D'), &ChipId, 4);
//    UpdateNameAslCode(SIGNATURE_32('S', 'I', 'D', 'V'), &ChipVer, 4);
//    UpdateNameAslCode(SIGNATURE_32('S', 'I', 'D', 'S'), ChipName, AsciiStrLen(ChipName)+1);
//    UpdateNameAslCode(SIGNATURE_32('P', 'L', 'S', 'T'), &PlatformSubtype, 4);
//    UpdateNameAslCode(SIGNATURE_32('S', 'O', 'S', 'N'), &Uid, 8);
//}
//

VOID
EFIAPI
PlatformSetup(VOID)
{
    // 1. 设置 CPU 最高性能（当前占位）
    SetupSprdCpuPerfLevels();

    // 2. 更新 ACPI 表
    //UpdateAcpiTablesForSprd();
}