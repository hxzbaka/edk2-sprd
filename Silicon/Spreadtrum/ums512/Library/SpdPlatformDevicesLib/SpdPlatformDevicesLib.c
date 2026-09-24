/** @file
  UMS512 platform setup: CPU performance, ACPI identity, and AP eFuse UID.
**/

#include <Uefi.h>
#include <Protocol/EFIChipInfo.h>
#include <Protocol/EFIClock.h>
#include <Library/AslUpdateLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define UMS512_CPU_COUNT       8U
#define AP_EFUSE_BASE           0x32240000U
#define AP_EFUSE_DATA_RD        0x00U
#define AP_EFUSE_BLOCK_INDEX    0x08U
#define AP_EFUSE_MODE_CTRL      0x0cU
#define AP_EFUSE_CFG0           0x10U
#define AP_EFUSE_STATUS         0x20U
#define AP_EFUSE_READ_START     BIT1
#define AP_EFUSE_READ_BUSY      BIT1
#define AP_EFUSE_VDD_ENABLE     BIT30
#define AP_EFUSE_VDDQ_K2_ON     BIT29
#define AP_EFUSE_READ_TIMEOUT   300000U
#define UMS512_UID_LOW_BLOCK    94U
#define UMS512_UID_HIGH_BLOCK   95U

/**
STATIC EFI_STATUS
GetSprdUid (OUT UINT64 *Uid)
{
  EFI_CLOCK_PROTOCOL *Clock;
  EFI_STATUS Status;
  UINTN ClockId;
  UINTN Attempt;
  UINT32 Config;
  UINT32 Low;
  UINT32 High;

  if (Uid == NULL) return EFI_INVALID_PARAMETER;
  Status = gBS->LocateProtocol (&gEfiClockProtocolGuid, NULL, (VOID **)&Clock);
  if (EFI_ERROR (Status)) return Status;
  Status = Clock->GetClockID (Clock, "efuse", &ClockId);
  if (EFI_ERROR (Status)) return Status;
  Status = Clock->EnableClock (Clock, ClockId);
  if (EFI_ERROR (Status)) return Status;
  Config = MmioRead32 (AP_EFUSE_BASE + AP_EFUSE_CFG0);
  Config &= ~BIT28;
  Config |= AP_EFUSE_VDD_ENABLE | AP_EFUSE_VDDQ_K2_ON;
  MmioWrite32 (AP_EFUSE_BASE + AP_EFUSE_CFG0, Config);
  gBS->Stall (1000U);
  MmioWrite32 (AP_EFUSE_BASE + AP_EFUSE_BLOCK_INDEX, UMS512_UID_LOW_BLOCK);
  MmioWrite32 (AP_EFUSE_BASE + AP_EFUSE_MODE_CTRL, AP_EFUSE_READ_START);
  for (Attempt = 0; Attempt < AP_EFUSE_READ_TIMEOUT; Attempt++) {
    if ((MmioRead32 (AP_EFUSE_BASE + AP_EFUSE_STATUS) & AP_EFUSE_READ_BUSY) == 0) break;
    gBS->Stall (1);
  }
  if (Attempt == AP_EFUSE_READ_TIMEOUT) Status = EFI_TIMEOUT;
  Low = MmioRead32 (AP_EFUSE_BASE + AP_EFUSE_DATA_RD);
  MmioWrite32 (AP_EFUSE_BASE + AP_EFUSE_BLOCK_INDEX, UMS512_UID_HIGH_BLOCK);
  MmioWrite32 (AP_EFUSE_BASE + AP_EFUSE_MODE_CTRL, AP_EFUSE_READ_START);
  for (Attempt = 0; Attempt < AP_EFUSE_READ_TIMEOUT; Attempt++) {
    if ((MmioRead32 (AP_EFUSE_BASE + AP_EFUSE_STATUS) & AP_EFUSE_READ_BUSY) == 0) break;
    gBS->Stall (1);
  }
  if (Attempt == AP_EFUSE_READ_TIMEOUT) Status = EFI_TIMEOUT;
  High = MmioRead32 (AP_EFUSE_BASE + AP_EFUSE_DATA_RD);
  Config = MmioRead32 (AP_EFUSE_BASE + AP_EFUSE_CFG0);
  Config &= ~AP_EFUSE_VDD_ENABLE;
  MmioWrite32 (AP_EFUSE_BASE + AP_EFUSE_CFG0, Config);
  gBS->Stall (1000U);
  Clock->DisableClock (Clock, ClockId);
  if (EFI_ERROR (Status)) return Status;
  *Uid = ((UINT64)High << 32) | Low;
  return EFI_SUCCESS;
}
**/

STATIC VOID
SetupSprdCpuPerfLevels (VOID)
{
  EFI_CLOCK_PROTOCOL *Clock;
  EFI_STATUS Status;
  UINT32 Cpu;
  UINT32 PerfLevel;
  UINT32 FrequencyHz;

  Status = gBS->LocateProtocol (&gEfiClockProtocolGuid, NULL, (VOID **)&Clock);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "SPRD: Clock protocol unavailable: %r\n", Status));
    return;
  }
  for (Cpu = 0; Cpu < UMS512_CPU_COUNT; Cpu++) {
    Status = Clock->GetMaxPerfLevel (Clock, Cpu, &PerfLevel);
    if (Status == EFI_UNSUPPORTED) {
      DEBUG ((DEBUG_INFO, "SPRD: CPU DVFS backend is unavailable; retaining boot frequency\n"));
      return;
    }
    if (EFI_ERROR (Status)) continue;
    Status = Clock->SetCpuPerfLevel (Clock, Cpu, PerfLevel, &FrequencyHz);
    DEBUG ((EFI_ERROR (Status) ? DEBUG_WARN : DEBUG_INFO,
      "SPRD: CPU%u perf level %u: %r, %u Hz\n", Cpu, PerfLevel, Status, FrequencyHz));
  }
}

/**
STATIC VOID
UpdateAcpiTablesForSprd (VOID)
{
  EFI_CHIPINFO_PROTOCOL *ChipInfo;
  EFI_STATUS Status;
  EFIChipInfoIdType ChipId;
  EFIChipInfoVersionType ChipVersion;
  EFIChipInfoFamilyType ChipFamily;
  EFIChipInfoModemType Modem;
  UINT64 Uid;
  BOOLEAN UidValid;

  Status = gBS->LocateProtocol (&gEfiChipInfoProtocolGuid, NULL, (VOID **)&ChipInfo);
  if (EFI_ERROR (Status)) return;
  if (EFI_ERROR (ChipInfo->GetChipId (ChipInfo, &ChipId)) ||
      EFI_ERROR (ChipInfo->GetChipVersion (ChipInfo, &ChipVersion)) ||
      EFI_ERROR (ChipInfo->GetChipFamily (ChipInfo, &ChipFamily)) ||
      EFI_ERROR (ChipInfo->GetModemSupport (ChipInfo, &Modem))) return;
  Status = GetSprdUid (&Uid);
  UidValid = !EFI_ERROR (Status);
  if (!UidValid) {
    DEBUG ((DEBUG_WARN, "SPRD-ACPI: AP eFuse UID unavailable: %r\n", Status));
  }
  UpdateNameAslCode (SIGNATURE_32 ('S','O','I','D'), &ChipId, sizeof (ChipId));
  UpdateNameAslCode (SIGNATURE_32 ('S','I','D','V'), &ChipVersion, sizeof (ChipVersion));
  UpdateNameAslCode (SIGNATURE_32 ('S','D','F','E'), &ChipFamily, sizeof (ChipFamily));
  UpdateNameAslCode (SIGNATURE_32 ('S','I','D','M'), &Modem, sizeof (Modem));
  if (UidValid) {
    UpdateNameAslCode (SIGNATURE_32 ('S','O','S','N'), &Uid, sizeof (Uid));
    DEBUG ((DEBUG_INFO, "SPRD-ACPI: id=%u version=0x%08x uid=0x%016lx\n", ChipId, ChipVersion, Uid));
  } else {
    DEBUG ((DEBUG_INFO, "SPRD-ACPI: id=%u version=0x%08x uid=unchanged\n", ChipId, ChipVersion));
  }
}
**/

VOID EFIAPI PlatformSetup (VOID) {
  SetupSprdCpuPerfLevels ();
//  UpdateAcpiTablesForSprd ();
}
