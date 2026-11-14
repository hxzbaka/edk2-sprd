/** @file
  SprdChipInfoDxe.c

  UEFI Chip Info driver for Unisoc UMS512.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

// ----------------------------------------------------------------------
// Local definitions from EFIChipInfoTypes.h (customized for UMS512)
// ----------------------------------------------------------------------

#define EFI_CHIPINFO_VERSION(major, minor)  (((major) << 16) | (minor))
#define EFI_CHIPINFO_VERSION_UNKNOWN        0

#define EFICHIPINFO_MAX_ID_LENGTH           16
#define EFICHIPINFO_MAX_NAME_LENGTH         EFICHIPINFO_MAX_ID_LENGTH
#define EFICHIPINFO_MAX_CPU_CLUSTERS        4

#define EFICHIPINFO_RAW_VERSION_UNKNOWN       0x0
#define EFICHIPINFO_RAW_ID_UNKNOWN            0x0
#define EFICHIPINFO_SERIAL_NUM_UNKNOWN        0x0
#define EFICHIPINFO_RAW_DEVICE_FAMILY_UNKNOWN 0x0
#define EFICHIPINFO_RAW_DEVICE_NUMBER_UNKNOWN 0x0
#define EFICHIPINFO_QFPROM_CHIPID_UNKNOWN     0x0

typedef UINT32 EFIChipInfoVersionType;
typedef UINT32 EFIChipInfoModemType;

typedef enum {
  EFICHIPINFO_ID_UNKNOWN = 0,
  EFICHIPINFO_ID_MSM8998 = 292,
  EFICHIPINFO_ID_APQ8098 = 319,
  EFICHIPINFO_ID_SDM845  = 321,
  EFICHIPINFO_ID_SC9832E = 322,
  EFICHIPINFO_ID_UMS512  = 326,
  EFICHIPINFO_NUM_IDS = 401,
  EFICHIPINFO_ID_32BITS = 0x7FFFFFF
} EFIChipInfoIdType;

typedef enum {
  EFICHIPINFO_FAMILY_UNKNOWN     = 0,
  EFICHIPINFO_FAMILY_MSM8998     = 67,
  EFICHIPINFO_FAMILY_SDM845      = 79,
  EFICHIPINFO_FAMILY_SPREADTRUM  = 100,    // Spreadtrum/Unisoc family
  EFICHIPINFO_NUM_FAMILIES       = 102,
  EFICHIPINFO_FAMILY_32BITS      = 0x7FFFFFF
} EFIChipInfoFamilyType;

typedef UINT32 EFIChipInfoSerialNumType;
typedef UINT32 EFIChipInfoQFPROMChipIdType;

typedef enum {
  EFICHIPINFO_FOUNDRYID_UNKNOWN = 0,
  EFICHIPINFO_FOUNDRYID_TSMC    = 1,
  EFICHIPINFO_FOUNDRYID_GF      = 2,
  EFICHIPINFO_FOUNDRYID_SS      = 3,
  EFICHIPINFO_FOUNDRYID_IBM     = 4,
  EFICHIPINFO_FOUNDRYID_UMC     = 5,
  EFICHIPINFO_FOUNDRYID_SMIC    = 6,
  EFICHIPINFO_NUM_FOUNDRYIDS,
  EFICHIPINFO_FOUNDRYID_32BITS  = 0x7FFFFFF
} EFIChipInfoFoundryIdType;

typedef enum {
  EFICHIPINFO_PART_UNKNOWN      = 0,
  EFICHIPINFO_PART_GPU          = 1,
  EFICHIPINFO_PART_VIDEO        = 2,
  EFICHIPINFO_PART_CAMERA       = 3,
  EFICHIPINFO_PART_DISPLAY      = 4,
  EFICHIPINFO_PART_AUDIO        = 5,
  EFICHIPINFO_PART_MODEM        = 6,
  EFICHIPINFO_PART_WLAN         = 7,
  EFICHIPINFO_PART_COMP         = 8,
  EFICHIPINFO_PART_SENSORS      = 9,
  EFICHIPINFO_PART_NPU          = 10,
  EFICHIPINFO_PART_SPSS         = 11,
  EFICHIPINFO_PART_NAV          = 12,
  EFICHIPINFO_NUM_PARTS,
  EFICHIPINFO_PART_32BITS = 0x7FFFFFFF
} EFIChipInfoPartType;

// ----------------------------------------------------------------------
// Protocol definitions (from EFIChipInfo.h)
// ----------------------------------------------------------------------

#define EFI_CHIPINFO_PROTOCOL_REVISION  0x0000000000010002

typedef struct _EFI_CHIPINFO_PROTOCOL EFI_CHIPINFO_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETCHIPVERSION) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT EFIChipInfoVersionType *pnVersion);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETRAWCHIPVERSION) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT UINT32                 *pnVersion);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETCHIPID) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT EFIChipInfoIdType      *peId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETRAWCHIPID) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT UINT32                 *pnId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETCHIPIDSTRING) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT CHAR8                  *szIdString,
    IN  UINT32                 nMaxLength);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETCHIPFAMILY) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT EFIChipInfoFamilyType  *peFamily);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETMODEMSUPPORT) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT EFIChipInfoModemType   *pnModem);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETPROCESSORNAMESTRING) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT CHAR8                  *szNameString,
    IN  UINT32                 nMaxLength);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETSERIALNUMBER) (
    IN  EFI_CHIPINFO_PROTOCOL   *This,
    OUT EFIChipInfoSerialNumType *peId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETFOUNDRYID) (
    IN  EFI_CHIPINFO_PROTOCOL     *This,
    OUT EFIChipInfoFoundryIdType  *peId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETRAWDEVICEFAMILY) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT UINT32                 *pnId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETRAWDEVICENUMBER) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT UINT32                 *pnId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETQFPROMCHIPID) (
    IN  EFI_CHIPINFO_PROTOCOL        *This,
    OUT EFIChipInfoQFPROMChipIdType  *pnId);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETMARKETINGNAMESTRING) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    OUT CHAR8                  *szNameString,
    IN  UINT32                 nMaxLength);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETDEFECTIVEPART) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    IN  EFIChipInfoPartType    ePart,
    OUT UINT32                 *pnMask);

typedef EFI_STATUS (EFIAPI *EFI_DALCHIPINFO_GETDEFECTIVECPUS) (
    IN  EFI_CHIPINFO_PROTOCOL  *This,
    IN  UINT32                 nCPUCluster,
    OUT UINT32                 *pnMask);

struct _EFI_CHIPINFO_PROTOCOL {
  UINT64                                    Revision;
  EFI_DALCHIPINFO_GETCHIPVERSION            GetChipVersion;
  EFI_DALCHIPINFO_GETRAWCHIPVERSION         GetRawChipVersion;
  EFI_DALCHIPINFO_GETCHIPID                 GetChipId;
  EFI_DALCHIPINFO_GETRAWCHIPID              GetRawChipId;
  EFI_DALCHIPINFO_GETCHIPIDSTRING           GetChipIdString;
  EFI_DALCHIPINFO_GETCHIPFAMILY             GetChipFamily;
  EFI_DALCHIPINFO_GETMODEMSUPPORT           GetModemSupport;
  EFI_DALCHIPINFO_GETPROCESSORNAMESTRING    GetProcessorNameString;
  EFI_DALCHIPINFO_GETSERIALNUMBER           GetSerialNumber;
  EFI_DALCHIPINFO_GETFOUNDRYID              GetFoundryId;
  EFI_DALCHIPINFO_GETRAWDEVICEFAMILY        GetRawDeviceFamily;
  EFI_DALCHIPINFO_GETRAWDEVICENUMBER        GetRawDeviceNumber;
  EFI_DALCHIPINFO_GETQFPROMCHIPID           GetQFPROMChipId;
  EFI_DALCHIPINFO_GETMARKETINGNAMESTRING    GetMarketingNameString;
  EFI_DALCHIPINFO_GETDEFECTIVEPART          GetDefectivePart;
  EFI_DALCHIPINFO_GETDEFECTIVECPUS          GetDefectiveCPUs;
};

// ----------------------------------------------------------------------
// Hardcoded UMS512 data
// ----------------------------------------------------------------------

#define UMS512_HARDWARE_ID      512        // from sc-id first cell
#define UMS512_VERSION_MAJOR    1          // from sc-id second cell
#define UMS512_VERSION_MINOR    0
#define UMS512_CHIP_NAME        "UMS512"
#define UMS512_PROCESSOR_NAME   "UMS512"

// ----------------------------------------------------------------------
// Protocol implementation
// ----------------------------------------------------------------------

EFI_STATUS EFIAPI
GetChipVersion (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT EFIChipInfoVersionType *pnVersion
  )
{
  *pnVersion = EFI_CHIPINFO_VERSION (UMS512_VERSION_MAJOR, UMS512_VERSION_MINOR);
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetRawChipVersion (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT UINT32                 *pnVersion
  )
{
  *pnVersion = EFI_CHIPINFO_VERSION (UMS512_VERSION_MAJOR, UMS512_VERSION_MINOR);
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetChipId (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT EFIChipInfoIdType      *peId
  )
{
  *peId = EFICHIPINFO_ID_UMS512;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetRawChipId (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT UINT32                 *pnId
  )
{
  *pnId = UMS512_HARDWARE_ID;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetChipIdString (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT CHAR8                  *szIdString,
  IN  UINT32                 nMaxLength
  )
{
  if (nMaxLength < AsciiStrLen (UMS512_CHIP_NAME) + 1)
    return EFI_BUFFER_TOO_SMALL;
  AsciiStrCpyS (szIdString, nMaxLength, UMS512_CHIP_NAME);
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetChipFamily (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT EFIChipInfoFamilyType  *peFamily
  )
{
  *peFamily = EFICHIPINFO_FAMILY_SPREADTRUM;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetModemSupport (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT EFIChipInfoModemType   *pnModem
  )
{
  *pnModem = 0;   // No modem or unknown
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetProcessorNameString (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT CHAR8                  *szNameString,
  IN  UINT32                 nMaxLength
  )
{
  if (nMaxLength < AsciiStrLen (UMS512_PROCESSOR_NAME) + 1)
    return EFI_BUFFER_TOO_SMALL;
  AsciiStrCpyS (szNameString, nMaxLength, UMS512_PROCESSOR_NAME);
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetSerialNumber (
  IN  EFI_CHIPINFO_PROTOCOL   *This,
  OUT EFIChipInfoSerialNumType *peId
  )
{
  *peId = EFICHIPINFO_SERIAL_NUM_UNKNOWN;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetFoundryId (
  IN  EFI_CHIPINFO_PROTOCOL     *This,
  OUT EFIChipInfoFoundryIdType  *peId
  )
{
  *peId = EFICHIPINFO_FOUNDRYID_UNKNOWN;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetRawDeviceFamily (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT UINT32                 *pnId
  )
{
  *pnId = EFICHIPINFO_RAW_DEVICE_FAMILY_UNKNOWN;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetRawDeviceNumber (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT UINT32                 *pnId
  )
{
  *pnId = EFICHIPINFO_RAW_DEVICE_NUMBER_UNKNOWN;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetQFPROMChipId (
  IN  EFI_CHIPINFO_PROTOCOL        *This,
  OUT EFIChipInfoQFPROMChipIdType  *pnId
  )
{
  *pnId = EFICHIPINFO_QFPROM_CHIPID_UNKNOWN;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
GetMarketingNameString (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  OUT CHAR8                  *szNameString,
  IN  UINT32                 nMaxLength
  )
{
  return GetProcessorNameString (This, szNameString, nMaxLength);
}

EFI_STATUS EFIAPI
GetDefectivePart (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  IN  EFIChipInfoPartType    ePart,
  OUT UINT32                 *pnMask
  )
{
  *pnMask = 0;
  return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI
GetDefectiveCPUs (
  IN  EFI_CHIPINFO_PROTOCOL  *This,
  IN  UINT32                 nCPUCluster,
  OUT UINT32                 *pnMask
  )
{
  *pnMask = 0;
  return EFI_NOT_FOUND;
}

// ----------------------------------------------------------------------
// Driver entry point
// ----------------------------------------------------------------------

EFI_STATUS
EFIAPI
InitializeSprdChipInfo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;
  EFI_CHIPINFO_PROTOCOL     Protocol;

  Protocol.Revision                = EFI_CHIPINFO_PROTOCOL_REVISION;
  Protocol.GetChipVersion          = GetChipVersion;
  Protocol.GetRawChipVersion       = GetRawChipVersion;
  Protocol.GetChipId               = GetChipId;
  Protocol.GetRawChipId            = GetRawChipId;
  Protocol.GetChipIdString         = GetChipIdString;
  Protocol.GetChipFamily           = GetChipFamily;
  Protocol.GetModemSupport         = GetModemSupport;
  Protocol.GetProcessorNameString  = GetProcessorNameString;
  Protocol.GetSerialNumber         = GetSerialNumber;
  Protocol.GetFoundryId            = GetFoundryId;
  Protocol.GetRawDeviceFamily      = GetRawDeviceFamily;
  Protocol.GetRawDeviceNumber      = GetRawDeviceNumber;
  Protocol.GetQFPROMChipId         = GetQFPROMChipId;
  Protocol.GetMarketingNameString  = GetMarketingNameString;
  Protocol.GetDefectivePart        = GetDefectivePart;
  Protocol.GetDefectiveCPUs        = GetDefectiveCPUs;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiChipInfoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &Protocol
                  );
  ASSERT_EFI_ERROR (Status);
  return Status;
}