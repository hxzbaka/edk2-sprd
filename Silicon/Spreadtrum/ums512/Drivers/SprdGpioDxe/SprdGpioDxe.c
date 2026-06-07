/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 hxzbaka
 *
 * Gpio/Adi/PMIC driver for Spreadtrum Devices
 * SC2721/SC2720/SC2731/SC2723T
 */

#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#include "SprdAdi.h"
#include <Protocol/SprdGpioProtocol.h>

#define AP_GPIO_BASE        0x40280000
#define AP_EIC_BASE         0x40210000

#define PMIC_EIC_BASE       0x280

#define GPIO_GROUP_NR       16
#define EIC_GROUP_NR        8

#define REG_DATA            0x00
#define REG_DMSK            0x04
#define REG_DIR             0x08
#define REG_INEN            0x28

#define GPIO_GROUP_OFFSET   0x80

STATIC
UINT32
ApGpioAddr (
  UINT32 Gpio,
  UINT32 Reg
  )
{
  UINT32 Group;

  Group = Gpio / GPIO_GROUP_NR;

  return AP_GPIO_BASE +
         Group * GPIO_GROUP_OFFSET +
         Reg;
}

STATIC
EFI_STATUS
ApGpioRequest (
  UINT32 Gpio
  )
{
  UINT32 Bit;

  Bit = 1U << (Gpio % GPIO_GROUP_NR);

  MmioOr32 (
    ApGpioAddr (Gpio, REG_DMSK),
    Bit
  );

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
ApGpioGet (
  UINT32 Gpio
  )
{
  UINT32 GroupBit;
  UINT32 Value;

  ApGpioRequest (Gpio);

  GroupBit = Gpio % GPIO_GROUP_NR;

  Value = MmioRead32 (
            ApGpioAddr (Gpio, REG_DATA)
          );

  return (BOOLEAN)((Value >> GroupBit) & 1);
}

STATIC
EFI_STATUS
ApGpioSet (
  UINT32 Gpio,
  BOOLEAN Level
  )
{
  UINT32 Bit;

  Bit = 1U << (Gpio % GPIO_GROUP_NR);

  if (Level) {
    MmioOr32 (
      ApGpioAddr (Gpio, REG_DATA),
      Bit
    );
  } else {
    MmioAnd32 (
      ApGpioAddr (Gpio, REG_DATA),
      ~Bit
    );
  }

  return EFI_SUCCESS;
}

STATIC
UINT32
ApEicAddr (
  UINT32 Eic,
  UINT32 Reg
  )
{
  UINT32 Group;

  Group = Eic / EIC_GROUP_NR;

  return AP_EIC_BASE +
         Group * GPIO_GROUP_OFFSET +
         Reg;
}

STATIC
EFI_STATUS
ApEicRequest (
  UINT32 Eic
  )
{
  UINT32 Bit;

  Bit = 1U << (Eic % EIC_GROUP_NR);

  MmioOr32 (
    ApEicAddr (Eic, REG_DMSK),
    Bit
  );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ApGpioDirectionInput (
  UINT32 Gpio
  )
{
  UINT32 Bit;

  ApGpioRequest (Gpio);

  Bit = 1U << (Gpio % GPIO_GROUP_NR);

  MmioAnd32 (
    ApGpioAddr (Gpio, REG_DIR),
    ~Bit
  );

  MmioOr32 (
    ApGpioAddr (Gpio, REG_INEN),
    Bit
  );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ApGpioDirectionOutput (
  UINT32 Gpio,
  BOOLEAN Level
  )
{
  UINT32 Bit;

  ApGpioRequest (Gpio);

  Bit = 1U << (Gpio % GPIO_GROUP_NR);

  ApGpioSet (Gpio, Level);

  MmioAnd32 (
    ApGpioAddr (Gpio, REG_INEN),
    ~Bit
  );

  MmioOr32 (
    ApGpioAddr (Gpio, REG_DIR),
    Bit
  );

  return EFI_SUCCESS;
}

//
// AP EIC
//

STATIC
EFI_STATUS
ApEicDirectionInput (
  UINT32 Eic
  )
{
  return ApEicRequest (Eic);
}

STATIC
BOOLEAN
ApEicGet (
  UINT32 Eic
  )
{
  UINT32 GroupBit;
  UINT32 Val;

  GroupBit = Eic % EIC_GROUP_NR;

  ApEicRequest (Eic);

  Val = MmioRead32 (
          ApEicAddr (Eic, REG_DATA)
        );

  return (BOOLEAN)((Val >> GroupBit) & 1);
}

//
// PMIC EIC
//

STATIC
EFI_STATUS
PmicEicRequest (
  UINT32 Eic
  )
{
  UINT16 Mask;

  if (Eic >= 16) {
    return EFI_INVALID_PARAMETER;
  }

  Mask = AdiRead (PMIC_EIC_BASE + REG_DMSK);

  Mask |= (UINT16)(1U << Eic);

  AdiWrite (
    PMIC_EIC_BASE + REG_DMSK,
    Mask
  );

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
PmicEicGet (
  UINT32 Eic
  )
{
  UINT16 Data;

  if (Eic >= 16) {
    return FALSE;
  }

  PmicEicRequest (Eic);

  Data = AdiRead (PMIC_EIC_BASE + REG_DATA);

  return (BOOLEAN)((Data >> Eic) & 1);
}

STATIC
EFI_STATUS
PmicEicDirectionInput (
  UINT32 Eic
  )
{
  return PmicEicRequest (Eic);
}

STATIC
EFI_STATUS
EFIAPI
SprdSet (
  UINT32 Gpio,
  BOOLEAN Value
  )
{
  if (Gpio >= 0x100) {
    return EFI_UNSUPPORTED;
  }

  return ApGpioSet (Gpio, Value);
}

STATIC
BOOLEAN
EFIAPI
SprdGet (
  UINT32 Gpio
  )
{
  if (Gpio < 0x100) {
    return ApGpioGet (Gpio);
  }

  if (Gpio >= 0x120 &&
      Gpio < 0x140) {
    return ApEicGet (Gpio - 0x120);
  }

  if (Gpio >= 0x140 &&
      Gpio < 0x150) {
    return PmicEicGet (Gpio - 0x140);
  }

  return FALSE;
}

STATIC
EFI_STATUS
EFIAPI
SprdDirectionInput (
  UINT32 Gpio
  )
{
  if (Gpio < 0x100) {
    return ApGpioDirectionInput (Gpio);
  }

  if (Gpio >= 0x120 &&
      Gpio < 0x140) {
    return ApEicDirectionInput (Gpio - 0x120);
  }

  if (Gpio >= 0x140 &&
      Gpio < 0x150) {
    return PmicEicDirectionInput (Gpio - 0x140);
  }

  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
SprdDirectionOutput (
  UINT32 Gpio,
  BOOLEAN Value
  )
{
  if (Gpio >= 0x100) {
    return EFI_INVALID_PARAMETER;
  }

  return ApGpioDirectionOutput (Gpio, Value);
}

STATIC SPRD_GPIO_PROTOCOL mSprdGpio = {
  SPRD_GPIO_PROTOCOL_REVISION,
  SprdDirectionInput,
  SprdDirectionOutput,
  SprdGet,
  SprdSet
};

EFI_STATUS
EFIAPI
SprdGpioEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HANDLE Handle = NULL;

  return gBS->InstallProtocolInterface (
                &Handle,
                &gSprdGpioProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &mSprdGpio
              );

  DEBUG ((
    DEBUG_ERROR,
    "[SprdGpioDxe] Load successful\n"
  ));
}