/** @file
  Spreadtrum ADI register access implementation.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
**/

#include "SprdAdi.h"
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/TimerLib.h>

// DTS: spi@32100000
#define ADI_BASE            0x32100000
#define ADI_SLAVE_BASE      0x32120000

#define REG_ADI_RD_CMD      0x28
#define REG_ADI_RD_DATA     0x2C
#define REG_ADI_FIFO_STS    0x30
#define BIT_RD_BUSY         BIT31
#define BIT_FIFO_EMPTY      BIT10

/* U-Boot waits roughly 2 ms for a read and drains the write FIFO before a
   synchronous transaction.  Keep the same ordering in DXE. */
#define ADI_READ_TIMEOUT_US  2000
#define ADI_FIFO_TIMEOUT_US  2000

STATIC
VOID
AdiDrainWriteFifo (
  VOID
  )
{
  UINTN Elapsed;

  for (Elapsed = 0; Elapsed < ADI_FIFO_TIMEOUT_US; ++Elapsed) {
    if ((MmioRead32 (ADI_BASE + REG_ADI_FIFO_STS) & BIT_FIFO_EMPTY) != 0) {
      MemoryFence ();
      return;
    }
    MicroSecondDelay (1);
  }
}

STATIC
BOOLEAN
AdiReadRawChecked (
  UINT32 Addr,
  UINT16 *Value
  )
{
  UINT32 Data;
  UINTN Elapsed;

  if (Value == NULL) {
    return FALSE;
  }

  MmioWrite32 (ADI_BASE + REG_ADI_RD_CMD, ADI_SLAVE_BASE + Addr);
  MemoryFence ();

  /*
   * The ADI read result register may still contain the previous transaction
   * immediately after RD_CMD is written.  U-Boot's __adi_read() performs a
   * read/one-microsecond-delay cycle; keep the same settling interval before
   * accepting RD_DATA.
   */
  MicroSecondDelay (1);
  Data = 0;
  for (Elapsed = 0; Elapsed < ADI_READ_TIMEOUT_US; ++Elapsed) {
    Data = MmioRead32 (ADI_BASE + REG_ADI_RD_DATA);
    if ((Data & BIT_RD_BUSY) == 0) {
      break;
    }
    MicroSecondDelay (1);
  }

  if (Elapsed == ADI_READ_TIMEOUT_US) {
    return FALSE;
  }

  *Value = (UINT16)(Data & 0xFFFFU);
  MemoryFence ();
  return TRUE;
}

UINT16
AdiRead (
  UINT16 Reg
  )
{
  UINT16 Value;

  if (!AdiReadChecked (Reg, &Value)) {
    return 0;
  }
  return Value;
}

BOOLEAN
AdiReadChecked (
  UINT16 Reg,
  UINT16 *Value
  )
{
  return AdiReadRawChecked (Reg, Value);
}

VOID
AdiWrite (
  UINT16 Reg,
  UINT16 Value
  )
{
  /* UMS512 U-Boot enables ADI_15BIT_MODE: PMIC registers are mapped at
     0x32120000, not the legacy 0x32108000 write window. */
  MmioWrite32 (ADI_SLAVE_BASE + Reg, Value);
  MemoryFence ();
  AdiDrainWriteFifo ();
}

VOID
AdiSetBits (
  UINT16 Reg,
  UINT16 Bits
  )
{
  UINT16 Val;

  Val = AdiRead (Reg);
  AdiWrite (Reg, Val | Bits);
}

VOID
AdiClrBits (
  UINT16 Reg,
  UINT16 Bits
  )
{
  UINT16 Val;

  Val = AdiRead (Reg);
  AdiWrite (Reg, Val & ~Bits);
}
