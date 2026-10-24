#include "SprdAdi.h"
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/TimerLib.h>

// DTS: spi@32100000
#define ADI_BASE            0x32100000
#define ADI_SLAVE_BASE      0x32120000
#define ADI_READ_TIMEOUT_US 2000

#define REG_ADI_RD_CMD      0x28
#define REG_ADI_RD_DATA     0x2C
#define BIT_RD_BUSY         BIT31

STATIC
UINT32
AdiReadRaw (
  UINT32 Addr
  )
{
  UINTN Elapsed;
  UINT32 Data;

  MmioWrite32 (ADI_BASE + REG_ADI_RD_CMD, ADI_SLAVE_BASE + Addr);
  MemoryFence ();
  MicroSecondDelay (1);

  for (Elapsed = 0; Elapsed < ADI_READ_TIMEOUT_US; Elapsed++) {
    Data = MmioRead32 (ADI_BASE + REG_ADI_RD_DATA);
    if ((Data & BIT_RD_BUSY) == 0) {
      return Data;
    }
    MicroSecondDelay (1);
  }

  return 0;
}

UINT16
AdiRead (
  UINT16 Reg
  )
{
  UINT32 Data;

  Data = AdiReadRaw (Reg);
  return (UINT16)(Data & 0xFFFF);
}

VOID
AdiWrite (
  UINT16 Reg,
  UINT16 Value
  )
{
  MmioWrite32 (ADI_SLAVE_BASE + Reg, Value);
  MemoryFence ();
  MicroSecondDelay (1);
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
