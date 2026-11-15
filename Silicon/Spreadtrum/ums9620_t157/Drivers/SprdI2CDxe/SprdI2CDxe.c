/** @file
  SprdI2CDxe.c

  UMS512/SharkL5Pro AON hardware I2C DXE driver.
  t610.dts assigns charger@6a to this controller.  The ARM hardware channel
  supports a two-byte register write, or a one-byte register address followed
  by a one/two-byte read.  Those are the transactions used by FAN54015.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <SprdUsb.h>
#include <Library/DebugLib.h>


#define I2C_CTL                 0x000U
#define I2C_STATUS              0x014U
#define I2C_ADDR_DVD0           0x020U
#define I2C_ADDR_DVD1           0x024U
#define I2C_ADDR_STA0_DVD       0x028U
#define I2C_ADDR_RST            0x02CU
#define I2C_HW_CTL              0x030U
#define I2C_HW_CHNL_PRIL        0x034U
#define I2C_ARM_CMD_WR          0x134U
#define I2C_ARM_DAT_WR          0x138U
#define I2C_ARM_RD_CMD          0x13CU
#define I2C_ARM_RD_DATA         0x140U
#define I2C_ARM_DEBUG1          0x148U

#define I2C_DVD_OPT             BIT8
#define I2C_EN                  BIT2
#define I2C_INT_EN              BIT1
#define I2C_RX_ACK              BIT1
#define I2C_RST                 BIT0
#define I2C_ARM_RD_BUSY         BIT31
#define I2C_CHNL_PENDING        BIT6
#define I2C_SLAVE_ADDR_SHIFT    10U
#define I2C_REGISTER_SHIFT      2U
#define I2C_SOURCE_CLOCK        26000000U
#define I2C_BUS_FREQUENCY       400000U
#define I2C_WRITE_DELAY_US      1000U
#define I2C_READ_TIMEOUT_US     1000U
#define SPRD_I2C_DEVICE_SIGNATURE  SIGNATURE_32 ('S', 'I', '2', 'C')

typedef struct {
  UINT32                    Signature;
  EFI_I2C_MASTER_PROTOCOL  I2c;
  SPRD_AON_I2C_PROTOCOL    AonI2c;
  EFI_HANDLE               Handle;
  EFI_PHYSICAL_ADDRESS     Base;
} SPRD_I2C_DEVICE;

STATIC
VOID
SprdI2cResetHardware (
  IN SPRD_I2C_DEVICE  *Device
  )
{
  MmioWrite32 ((UINTN)Device->Base + I2C_ADDR_RST, I2C_RST);
  MicroSecondDelay (10);
}

STATIC
EFI_STATUS
SprdI2cCheckAck (
  IN SPRD_I2C_DEVICE  *Device
  )
{
  UINT32  Status;

  Status = MmioRead32 ((UINTN)Device->Base + I2C_STATUS);
  if ((Status & I2C_RX_ACK) == 0) {
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_ERROR, "SPRD-I2C: NACK/status error, STATUS=0x%08x\n", Status));
  SprdI2cResetHardware (Device);
  MmioWrite32 ((UINTN)Device->Base + I2C_STATUS, Status & ~I2C_RX_ACK);
  return EFI_NO_RESPONSE;
}

STATIC
EFI_STATUS
SprdI2cWriteRegister (
  IN SPRD_I2C_DEVICE  *Device,
  IN UINTN            SlaveAddress,
  IN UINT8            Register,
  IN UINT8            Value
  )
{
  EFI_STATUS  Status;
  UINT32      Command;

  Status = SprdI2cCheckAck (Device);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Command = ((UINT32)SlaveAddress << I2C_SLAVE_ADDR_SHIFT) |
            ((UINT32)Register << I2C_REGISTER_SHIFT);
  MmioWrite32 ((UINTN)Device->Base + I2C_ARM_CMD_WR, Command);
  MmioWrite32 ((UINTN)Device->Base + I2C_ARM_DAT_WR, Value);
  MicroSecondDelay (I2C_WRITE_DELAY_US);
  if ((MmioRead32 ((UINTN)Device->Base + I2C_ARM_DEBUG1) & I2C_CHNL_PENDING) != 0) {
    DEBUG ((DEBUG_ERROR, "SPRD-I2C: write to 0x%02x reg 0x%02x did not complete\n",
      SlaveAddress, Register));
    SprdI2cResetHardware (Device);
    return EFI_DEVICE_ERROR;
  }
  return SprdI2cCheckAck (Device);
}

STATIC
EFI_STATUS
SprdI2cReadRegister (
  IN  SPRD_I2C_DEVICE  *Device,
  IN  UINTN            SlaveAddress,
  IN  UINT8            Register,
  OUT UINT8            *Buffer,
  IN  UINTN            Length
  )
{
  EFI_STATUS  Status;
  UINT32      Command;
  UINT32      Data;
  UINTN       Index;
  UINTN       Elapsed;

  Status = SprdI2cCheckAck (Device);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Command = ((UINT32)SlaveAddress << I2C_SLAVE_ADDR_SHIFT) |
            ((UINT32)Register << I2C_REGISTER_SHIFT);
  MmioWrite32 ((UINTN)Device->Base + I2C_ARM_RD_CMD, Command);
  for (Index = 0; Index < Length; Index++) {
    for (Elapsed = 0; Elapsed < I2C_READ_TIMEOUT_US; Elapsed++) {
      Data = MmioRead32 ((UINTN)Device->Base + I2C_ARM_RD_DATA);
      if ((Data & I2C_ARM_RD_BUSY) == 0) {
        break;
      }
      MicroSecondDelay (1);
    }
    if (Elapsed == I2C_READ_TIMEOUT_US) {
      DEBUG ((DEBUG_ERROR, "SPRD-I2C: read timeout, slave=0x%02x reg=0x%02x\n",
        SlaveAddress, Register));
      SprdI2cResetHardware (Device);
      return EFI_TIMEOUT;
    }
    Buffer[Index] = (UINT8)Data;
  }
  return SprdI2cCheckAck (Device);
}

STATIC
EFI_STATUS
EFIAPI
SprdI2cSetBusFrequency (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This,
  IN OUT UINTN                       *BusClockHertz
  )
{
  if ((BusClockHertz == NULL) || (*BusClockHertz != I2C_BUS_FREQUENCY)) {
    return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SprdI2cReset (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This
  )
{
  SPRD_I2C_DEVICE  *Device;

  Device = CR (This, SPRD_I2C_DEVICE, I2c, SPRD_I2C_DEVICE_SIGNATURE);
  SprdI2cResetHardware (Device);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SprdI2cStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This,
  IN UINTN                           SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET          *RequestPacket,
  IN EFI_EVENT                       Event OPTIONAL,
  OUT EFI_STATUS                     *I2cStatus OPTIONAL
  )
{
  SPRD_I2C_DEVICE   *Device;
  EFI_I2C_OPERATION *First;
  EFI_STATUS        Status;

  if ((RequestPacket == NULL) || (SlaveAddress > 0x7F) ||
      (RequestPacket->OperationCount == 0)) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  Device = CR (This, SPRD_I2C_DEVICE, I2c, SPRD_I2C_DEVICE_SIGNATURE);
  DEBUG ((DEBUG_VERBOSE, "SPRD-I2C: request slave=0x%02x operations=%u\n",
  SlaveAddress, RequestPacket->OperationCount));
  if (RequestPacket->OperationCount == 1) {
    First = &RequestPacket->Operation[0];
    if (((First->Flags & I2C_FLAG_READ) != 0) || (First->LengthInBytes != 2) ||
        (First->Buffer == NULL)) {
      Status = EFI_UNSUPPORTED;
    } else {
      Status = SprdI2cWriteRegister (Device, SlaveAddress, ((UINT8 *)First->Buffer)[0],
                 ((UINT8 *)First->Buffer)[1]);
    }
  } else if (RequestPacket->OperationCount == 2) {
    First = &RequestPacket->Operation[0];
    if (((First->Flags & I2C_FLAG_READ) != 0) || (First->LengthInBytes != 1) ||
        (First->Buffer == NULL) || ((RequestPacket->Operation[1].Flags & I2C_FLAG_READ) == 0) ||
        (RequestPacket->Operation[1].LengthInBytes == 0) ||
        (RequestPacket->Operation[1].LengthInBytes > 2) ||
        (RequestPacket->Operation[1].Buffer == NULL)) {
      Status = EFI_UNSUPPORTED;
    } else {
      Status = SprdI2cReadRegister (Device, SlaveAddress, ((UINT8 *)First->Buffer)[0],
                 RequestPacket->Operation[1].Buffer, RequestPacket->Operation[1].LengthInBytes);
    }
  } else {
    Status = EFI_UNSUPPORTED;
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-I2C: request to 0x%02x failed: %r\n", SlaveAddress, Status));
  }

Done:
  if (I2cStatus != NULL) {
    *I2cStatus = Status;
  }
  if (Event != NULL) {
    gBS->SignalEvent (Event);
    return EFI_SUCCESS;
  }
  return Status;
}

STATIC
VOID
SprdI2cConfigureHardware (
  IN SPRD_I2C_DEVICE  *Device
  )
{
  UINT32  Prescale;
  UINT32  High;
  UINT32  Low;

  /* AON gate 36 is i2c-eb; AON clock mux value 1 selects ext-26m. */
  MmioOr32 (SPRD_AON_APB_BASE + 0x0008U, BIT7);
  MmioWrite32 (SPRD_AON_CLK_BASE + 0x027CU,
    (MmioRead32 (SPRD_AON_CLK_BASE + 0x027CU) & ~0x7U) | 1U);

  Prescale = (I2C_SOURCE_CLOCK / (4U * I2C_BUS_FREQUENCY)) - 1U;
  High = ((Prescale << 1) * 2U) / 6U;
  Low = ((Prescale << 1) * 3U) / 6U;
  MmioWrite32 ((UINTN)Device->Base + I2C_CTL, I2C_DVD_OPT);
  MmioWrite32 ((UINTN)Device->Base + I2C_ADDR_DVD0, (High << 16) | Low);
  MmioWrite32 ((UINTN)Device->Base + I2C_ADDR_DVD1, 0);
  MmioWrite32 ((UINTN)Device->Base + I2C_ADDR_STA0_DVD, (6U * I2C_SOURCE_CLOCK) / 10000000U);
  MmioWrite32 ((UINTN)Device->Base + I2C_HW_CTL, 0x30U);
  MmioWrite32 ((UINTN)Device->Base + I2C_HW_CHNL_PRIL, 0x64U);
  /* The hardware ARM channel requires INT_EN even when completion is polled. */
  MmioOr32 ((UINTN)Device->Base + I2C_CTL, I2C_EN | I2C_INT_EN);
  DEBUG ((DEBUG_INFO, "SPRD-I2C: AON HW I2C at 0x%lx configured for %u Hz\n",
    Device->Base, I2C_BUS_FREQUENCY));
}

EFI_STATUS
EFIAPI
SprdI2CDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS       Status;
  SPRD_I2C_DEVICE  *Device;

  Device = AllocateZeroPool (sizeof (*Device));
  if (Device == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  Device->Base = SPRD_AON_HW_I2C_BASE;
  Device->Signature = SPRD_I2C_DEVICE_SIGNATURE;
  DEBUG ((DEBUG_INFO, "SPRD-I2C: starting AON I2C DXE\n"));
  SprdI2cConfigureHardware (Device);
  Device->I2c.SetBusFrequency = SprdI2cSetBusFrequency;
  Device->I2c.Reset = SprdI2cReset;
  Device->I2c.StartRequest = SprdI2cStartRequest;
  Device->AonI2c.I2c = &Device->I2c;
  Status = gBS->InstallMultipleProtocolInterfaces (&Device->Handle,
                  &gEfiI2cMasterProtocolGuid, &Device->I2c,
                  &gSprdAonI2cProtocolGuid, &Device->AonI2c,
                  NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-I2C: protocol installation failed: %r\n", Status));
    FreePool (Device);
  } else {
    DEBUG ((DEBUG_INFO, "SPRD-I2C: EFI_I2C_MASTER_PROTOCOL installed\n"));
  }
  return Status;
}