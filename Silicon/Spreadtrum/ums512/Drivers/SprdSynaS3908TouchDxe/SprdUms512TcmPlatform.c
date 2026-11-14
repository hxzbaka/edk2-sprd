/** @file
  SprdUms512TcmPlatform.c

  UMS512 touch-controller platform transport implementation.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SprdUms512TcmPlatform.h"

/* UMS512 sharkl5pro software I2C register map. */
#define SPRD_I2C_CTL                 0x00
#define SPRD_I2C_ADDR_CFG            0x04
#define SPRD_I2C_COUNT               0x08
#define SPRD_I2C_RX                  0x0c
#define SPRD_I2C_TX                  0x10
#define SPRD_I2C_STATUS              0x14
#define SPRD_I2C_ADDR_DVD0           0x20
#define SPRD_I2C_ADDR_DVD1           0x24
#define SPRD_I2C_ADDR_STA0_DVD       0x28
#define SPRD_I2C_ADDR_RST            0x2c

#define SPRD_I2C_STP_EN              BIT20
#define SPRD_I2C_FIFO_AF_LVL_SHIFT   16
#define SPRD_I2C_FIFO_AE_LVL_SHIFT   12
#define SPRD_I2C_FIFO_AF_LVL_MASK    (0xFU << SPRD_I2C_FIFO_AF_LVL_SHIFT)
#define SPRD_I2C_FIFO_AE_LVL_MASK    (0xFU << SPRD_I2C_FIFO_AE_LVL_SHIFT)
#define SPRD_I2C_FULL_INTEN          BIT10
#define SPRD_I2C_EMPTY_INTEN         BIT9
#define SPRD_I2C_DVD_OPT             BIT8
#define SPRD_I2C_MODE                BIT3
#define SPRD_I2C_EN                  BIT2
#define SPRD_I2C_INT_EN              BIT1
#define SPRD_I2C_START               BIT0

#define SPRD_I2C_INT                 BIT2
#define SPRD_I2C_RX_ACK              BIT1
#define SPRD_I2C_BUSY                BIT0
#define SPRD_I2C_FIFO_DEEP           12U
#define SPRD_I2C_FIFO_FULL_THLD      15U
#define SPRD_I2C_FIFO_EMPTY_THLD     4U
#define SPRD_I2C_TIMEOUT_STEP_US     10U

/* UMS512 AP GPIO register map. */
#define SPRD_GPIO_DATA               0x00
#define SPRD_GPIO_DMSK               0x04
#define SPRD_GPIO_DIR                0x08
#define SPRD_GPIO_INEN               0x28
#define SPRD_GPIO_BANK_SIZE          0x80U
#define SPRD_GPIO_BANK_NR            16U
#define SPRD_GPIO_MAX_LINES          256U

STATIC
UINTN
SprdGpioRegister (
  IN SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line,
  IN UINT32 Register
  )
{
  return Gpio->Base + ((Line / SPRD_GPIO_BANK_NR) * SPRD_GPIO_BANK_SIZE) + Register;
}

STATIC
EFI_STATUS
SprdGpioUpdateBit (
  IN SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line,
  IN UINT32 Register,
  IN BOOLEAN Set
  )
{
  UINT32 Value;

  if (Gpio == NULL || !Gpio->Ready || Line >= SPRD_GPIO_MAX_LINES) {
    return EFI_INVALID_PARAMETER;
  }

  Value = MmioRead32 (SprdGpioRegister (Gpio, Line, Register));
  if (Set) {
    Value |= (UINT32)1U << (Line % SPRD_GPIO_BANK_NR);
  } else {
    Value &= ~((UINT32)1U << (Line % SPRD_GPIO_BANK_NR));
  }
  MmioWrite32 (SprdGpioRegister (Gpio, Line, Register), Value);
  return EFI_SUCCESS;
}

STATIC
VOID
SprdI2cClearStartAndInterrupt (
  IN SPRD_UMS512_I2C *I2c
  )
{
  UINT32 Value;

  Value = MmioRead32 (I2c->Base + SPRD_I2C_CTL);
  Value &= ~(SPRD_I2C_START | SPRD_I2C_FULL_INTEN | SPRD_I2C_EMPTY_INTEN);
  MmioWrite32 (I2c->Base + SPRD_I2C_CTL, Value);

  Value = MmioRead32 (I2c->Base + SPRD_I2C_STATUS);
  Value &= ~(SPRD_I2C_INT | SPRD_I2C_RX_ACK);
  MmioWrite32 (I2c->Base + SPRD_I2C_STATUS, Value);
}

STATIC
VOID
SprdI2cResetFifo (
  IN SPRD_UMS512_I2C *I2c
  )
{
  MmioWrite32 (I2c->Base + SPRD_I2C_ADDR_RST, 1U);
}

STATIC
VOID
SprdI2cSetClock (
  IN SPRD_UMS512_I2C *I2c
  )
{
  UINT32 Divider;
  UINT32 High;
  UINT32 Low;
  UINT32 Div0;
  UINT32 Div1;

  Divider = I2c->SourceClockHz / (4U * I2c->BusFrequencyHz) - 1U;
  High = ((Divider << 1) * 2U) / 5U;
  Low = ((Divider << 1) * 3U) / 5U;
  Div0 = ((High & 0xffffU) << 16) | (Low & 0xffffU);
  Div1 = (High & 0xffff0000U) | ((Low & 0xffff0000U) >> 16);

  MmioWrite32 (I2c->Base + SPRD_I2C_ADDR_DVD0, Div0);
  MmioWrite32 (I2c->Base + SPRD_I2C_ADDR_DVD1, Div1);
  if (I2c->BusFrequencyHz == 400000U) {
    MmioWrite32 (
      I2c->Base + SPRD_I2C_ADDR_STA0_DVD,
      (6U * I2c->SourceClockHz) / 10000000U
      );
  } else {
    MmioWrite32 (
      I2c->Base + SPRD_I2C_ADDR_STA0_DVD,
      (4U * I2c->SourceClockHz) / 1000000U
      );
  }
}

STATIC
VOID
SprdI2cEnable (
  IN SPRD_UMS512_I2C *I2c
  )
{
  UINT32 Control;

  MmioWrite32 (I2c->Base + SPRD_I2C_CTL, SPRD_I2C_DVD_OPT);
  Control = MmioRead32 (I2c->Base + SPRD_I2C_CTL);
  Control &= ~(SPRD_I2C_FIFO_AF_LVL_MASK | SPRD_I2C_FIFO_AE_LVL_MASK);
  Control |= (15U << SPRD_I2C_FIFO_AF_LVL_SHIFT) |
             (SPRD_I2C_FIFO_EMPTY_THLD << SPRD_I2C_FIFO_AE_LVL_SHIFT);
  MmioWrite32 (I2c->Base + SPRD_I2C_CTL, Control);
  SprdI2cSetClock (I2c);
  SprdI2cResetFifo (I2c);
  SprdI2cClearStartAndInterrupt (I2c);

  Control = MmioRead32 (I2c->Base + SPRD_I2C_CTL);
  Control |= SPRD_I2C_EN | SPRD_I2C_INT_EN;
  MmioWrite32 (I2c->Base + SPRD_I2C_CTL, Control);
}

STATIC
BOOLEAN
SprdI2cTimedOut (
  IN UINT32 ElapsedUs
  )
{
  return ElapsedUs >= SPRD_UMS512_TCM_I2C_TIMEOUT_US;
}

EFI_STATUS
SprdUms512I2cInit (
  OUT SPRD_UMS512_I2C *I2c
  )
{
  if (I2c == NULL || SPRD_UMS512_TCM_I2C_BASE == 0U ||
      SPRD_UMS512_TCM_I2C_SOURCE_CLOCK == 0U ||
      SPRD_UMS512_TCM_I2C_ADDRESS > 0x7fU ||
      (SPRD_UMS512_TCM_I2C_BUS_FREQUENCY != 100000U &&
       SPRD_UMS512_TCM_I2C_BUS_FREQUENCY != 400000U)) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (I2c, sizeof (*I2c));
  I2c->Base = SPRD_UMS512_TCM_I2C_BASE;
  I2c->SourceClockHz = SPRD_UMS512_TCM_I2C_SOURCE_CLOCK;
  I2c->BusFrequencyHz = SPRD_UMS512_TCM_I2C_BUS_FREQUENCY;
  I2c->SlaveAddress = (UINT8)SPRD_UMS512_TCM_I2C_ADDRESS;

  /* AP I2C3 access aborts while its AP_APB gate is closed. */
  MmioOr32 (SPRD_UMS512_TCM_AP_APB_BASE, SPRD_UMS512_TCM_I2C3_EB_BIT);
  SprdI2cEnable (I2c);
  I2c->Ready = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
SprdUms512I2cTransfer (
  IN OUT SPRD_UMS512_I2C *I2c,
  IN BOOLEAN Read,
  OUT UINT8 *ReadBuffer OPTIONAL,
  IN CONST UINT8 *WriteBuffer OPTIONAL,
  IN UINTN Length
  )
{
  UINTN Offset;
  UINTN Remaining;
  UINTN Chunk;
  UINT32 Status;
  UINT32 Control;
  UINT32 ElapsedUs;
  BOOLEAN WaitingForCompletion;
  CONST UINT8 *WritePtr;
  UINT8 *ReadPtr;

  if (I2c == NULL || !I2c->Ready || Length == 0U ||
      (Read && ReadBuffer == NULL) || (!Read && WriteBuffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  SprdI2cResetFifo (I2c);
  SprdI2cClearStartAndInterrupt (I2c);
  MmioWrite32 (I2c->Base + SPRD_I2C_ADDR_CFG, (UINT32)I2c->SlaveAddress << 1);
  MmioWrite32 (I2c->Base + SPRD_I2C_COUNT, (UINT32)Length);

  Control = MmioRead32 (I2c->Base + SPRD_I2C_CTL);
  Control &= ~(SPRD_I2C_MODE | SPRD_I2C_STP_EN |
               SPRD_I2C_FULL_INTEN | SPRD_I2C_EMPTY_INTEN);
  if (Read) {
    Control |= SPRD_I2C_MODE | SPRD_I2C_FULL_INTEN;
  }
  /* The kernel driver clears STP_EN for the final (single) message. */
  MmioWrite32 (I2c->Base + SPRD_I2C_CTL, Control);

  Offset = 0U;
  Remaining = Length;
  WaitingForCompletion = TRUE;
  WritePtr = WriteBuffer;
  ReadPtr = ReadBuffer;
  if (!Read) {
    Chunk = MIN (Remaining, (UINTN)SPRD_I2C_FIFO_DEEP);
    while (Chunk-- != 0U) {
      MmioWrite8 (I2c->Base + SPRD_I2C_TX, WritePtr[Offset++]);
      Remaining--;
    }
    if (Remaining != 0U) {
      Control = MmioRead32 (I2c->Base + SPRD_I2C_CTL);
      Control |= SPRD_I2C_EMPTY_INTEN;
      MmioWrite32 (I2c->Base + SPRD_I2C_CTL, Control);
    }
  }

  Control = MmioRead32 (I2c->Base + SPRD_I2C_CTL);
  MmioWrite32 (I2c->Base + SPRD_I2C_CTL, Control | SPRD_I2C_START);

  ElapsedUs = 0U;
  while (WaitingForCompletion) {
    Status = MmioRead32 (I2c->Base + SPRD_I2C_STATUS);
    if ((Status & SPRD_I2C_RX_ACK) != 0U) {
      SprdI2cClearStartAndInterrupt (I2c);
      return EFI_DEVICE_ERROR;
    }

    if ((Status & SPRD_I2C_INT) == 0U) {
      if (SprdI2cTimedOut (ElapsedUs)) {
        SprdI2cClearStartAndInterrupt (I2c);
        return EFI_TIMEOUT;
      }
      gBS->Stall (SPRD_I2C_TIMEOUT_STEP_US);
      ElapsedUs += SPRD_I2C_TIMEOUT_STEP_US;
      continue;
    }

    if (Read) {
      Chunk = MIN (Remaining, (UINTN)SPRD_I2C_FIFO_FULL_THLD);
      while (Chunk-- != 0U) {
        ReadPtr[Offset++] = MmioRead8 (I2c->Base + SPRD_I2C_RX);
        Remaining--;
      }
    } else {
      Chunk = MIN (Remaining, (UINTN)SPRD_I2C_FIFO_DEEP);
      while (Chunk-- != 0U) {
        MmioWrite8 (I2c->Base + SPRD_I2C_TX, WritePtr[Offset++]);
        Remaining--;
      }
    }

    if (Remaining == 0U) {
      WaitingForCompletion = FALSE;
    }

    Status = MmioRead32 (I2c->Base + SPRD_I2C_STATUS);
    Status &= ~SPRD_I2C_INT;
    MmioWrite32 (I2c->Base + SPRD_I2C_STATUS, Status);
  }

  SprdI2cClearStartAndInterrupt (I2c);
  return EFI_SUCCESS;
}

EFI_STATUS
SprdUms512GpioInit (
  OUT SPRD_UMS512_GPIO *Gpio
  )
{
  if (Gpio == NULL || SPRD_UMS512_TCM_GPIO_BASE == 0U) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (Gpio, sizeof (*Gpio));
  Gpio->Base = SPRD_UMS512_TCM_GPIO_BASE;
  Gpio->Ready = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
SprdUms512GpioInput (
  IN OUT SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line
  )
{
  EFI_STATUS Status;

  Status = SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_DMSK, TRUE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_DIR, FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  return SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_INEN, TRUE);
}

EFI_STATUS
SprdUms512GpioOutput (
  IN OUT SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line,
  IN BOOLEAN Value
  )
{
  EFI_STATUS Status;

  Status = SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_DMSK, TRUE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_DIR, TRUE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_INEN, FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  return SprdGpioUpdateBit (Gpio, Line, SPRD_GPIO_DATA, Value);
}

EFI_STATUS
SprdUms512GpioGet (
  IN SPRD_UMS512_GPIO *Gpio,
  IN UINT32 Line,
  OUT BOOLEAN *Value
  )
{
  UINT32 Data;

  if (Gpio == NULL || !Gpio->Ready || Value == NULL ||
      Line >= SPRD_GPIO_MAX_LINES) {
    return EFI_INVALID_PARAMETER;
  }

  Data = MmioRead32 (SprdGpioRegister (Gpio, Line, SPRD_GPIO_DATA));
  *Value = ((Data & ((UINT32)1U << (Line % SPRD_GPIO_BANK_NR))) != 0U);
  return EFI_SUCCESS;
}

EFI_STATUS
SprdUms512TouchReset (
  IN OUT SPRD_UMS512_GPIO *Gpio
  )
{
  BOOLEAN Active;
  EFI_STATUS Status;

  /* Convert the logical active state to the electrical GPIO value. */
  Active = (SPRD_UMS512_TCM_RESET_ACTIVE_LOW == 0);
  Status = SprdUms512GpioOutput (Gpio, SPRD_UMS512_TCM_RESET_GPIO, Active);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  gBS->Stall (SPRD_UMS512_TCM_RESET_ACTIVE_US);
  Status = SprdUms512GpioOutput (Gpio, SPRD_UMS512_TCM_RESET_GPIO, !Active);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  gBS->Stall (SPRD_UMS512_TCM_RESET_SETTLE_US);
  return EFI_SUCCESS;
}