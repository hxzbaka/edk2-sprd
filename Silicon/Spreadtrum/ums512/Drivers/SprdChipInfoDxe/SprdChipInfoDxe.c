/** @file
  UEFI Chip Info driver for Unisoc UMS512.
**/

#include <Uefi.h>
#include <Protocol/EFIChipInfo.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>

#define UMS512_AON_APB_BASE   0x327D0000U
#define UMS512_AON_VER_ID     (UMS512_AON_APB_BASE + 0x00F8U)
#define UMS512_AON_CHIP_ID    (UMS512_AON_APB_BASE + 0x00FCU)
#define UMS512_CHIP_NAME      "UMS512"

STATIC EFI_STATUS
CopyChipName (OUT CHAR8 *Name, IN UINT32 NameSize)
{
  if ((Name == NULL) || (NameSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  if (NameSize < AsciiStrLen (UMS512_CHIP_NAME) + 1U) {
    return EFI_BUFFER_TOO_SMALL;
  }
  return AsciiStrCpyS (Name, NameSize, UMS512_CHIP_NAME);
}

STATIC EFI_STATUS EFIAPI GetChipVersion (IN EFI_CHIPINFO_PROTOCOL *This, OUT EFIChipInfoVersionType *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = MmioRead32 (UMS512_AON_VER_ID);
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetRawChipVersion (IN EFI_CHIPINFO_PROTOCOL *This, OUT UINT32 *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = MmioRead32 (UMS512_AON_VER_ID);
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetChipId (IN EFI_CHIPINFO_PROTOCOL *This, OUT EFIChipInfoIdType *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = EFICHIPINFO_ID_UMS512;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetRawChipId (IN EFI_CHIPINFO_PROTOCOL *This, OUT UINT32 *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = MmioRead32 (UMS512_AON_CHIP_ID);
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetChipIdString (IN EFI_CHIPINFO_PROTOCOL *This, OUT CHAR8 *Value, IN UINT32 Size) { return CopyChipName (Value, Size); }
STATIC EFI_STATUS EFIAPI GetChipFamily (IN EFI_CHIPINFO_PROTOCOL *This, OUT EFIChipInfoFamilyType *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = EFICHIPINFO_FAMILY_SPREADTRUM;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetModemSupport (IN EFI_CHIPINFO_PROTOCOL *This, OUT EFIChipInfoModemType *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = 0;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetProcessorName (IN EFI_CHIPINFO_PROTOCOL *This, OUT CHAR8 *Value, IN UINT32 Size) { return CopyChipName (Value, Size); }
STATIC EFI_STATUS EFIAPI GetUnknown32 (IN EFI_CHIPINFO_PROTOCOL *This, OUT UINT32 *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = 0;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetFoundryId (IN EFI_CHIPINFO_PROTOCOL *This, OUT EFIChipInfoFoundryIdType *Value) {
  if (Value == NULL) return EFI_INVALID_PARAMETER;
  *Value = EFICHIPINFO_FOUNDRYID_UNKNOWN;
  return EFI_SUCCESS;
}
STATIC EFI_STATUS EFIAPI GetMarketingName (IN EFI_CHIPINFO_PROTOCOL *This, OUT CHAR8 *Value, IN UINT32 Size) { return CopyChipName (Value, Size); }
STATIC EFI_STATUS EFIAPI GetDefectivePart (IN EFI_CHIPINFO_PROTOCOL *This, IN EFIChipInfoPartType Part, OUT UINT32 *Mask) {
  if (Mask == NULL) return EFI_INVALID_PARAMETER;
  *Mask = 0;
  if (Part >= EFICHIPINFO_NUM_PARTS) return EFI_NOT_FOUND;
  return EFI_NOT_FOUND;
}
STATIC EFI_STATUS EFIAPI GetDefectiveCpus (IN EFI_CHIPINFO_PROTOCOL *This, IN UINT32 Cluster, OUT UINT32 *Mask) {
  if (Mask == NULL) return EFI_INVALID_PARAMETER;
  *Mask = 0;
  if (Cluster >= EFICHIPINFO_MAX_CPU_CLUSTERS) return EFI_NOT_FOUND;
  return EFI_NOT_FOUND;
}

STATIC EFI_CHIPINFO_PROTOCOL mChipInfoProtocol = {
  EFI_CHIPINFO_PROTOCOL_REVISION,
  GetChipVersion, GetRawChipVersion, GetChipId, GetRawChipId,
  GetChipIdString, GetChipFamily, GetModemSupport, GetProcessorName,
  GetUnknown32, GetFoundryId, GetUnknown32, GetUnknown32,
  GetUnknown32, GetMarketingName, GetDefectivePart, GetDefectiveCpus
};

EFI_STATUS EFIAPI
InitializeSprdChipInfo (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_HANDLE Handle = NULL;
  EFI_STATUS Status;

  Status = gBS->InstallProtocolInterface (&Handle, &gEfiChipInfoProtocolGuid,
                  EFI_NATIVE_INTERFACE, &mChipInfoProtocol);
  if (EFI_ERROR (Status)) DEBUG ((DEBUG_ERROR, "SPRD-ChipInfo: install failed: %r\n", Status));
  return Status;
}
