#include "SprdAdi.h"
#include <Library/IoLib.h>

// DTS: spi@400000 (Y60/UMS9620, AON base 0x64000000)
#define ADI_BASE            0x64400000
// QOGIRN6PRO uses 15-bit channel offset (not 0x8000)
#define ADI_CHANNEL_OFFSET  0x20000

#define REG_ADI_RD_CMD      0x28
#define REG_ADI_RD_DATA     0x2C
#define BIT_RD_BUSY         BIT31

STATIC
UINT32
AdiReadRaw (
  UINT32 Addr
  )
{
  MmioWrite32 (ADI_BASE + REG_ADI_RD_CMD, Addr);

  while (MmioRead32 (ADI_BASE + REG_ADI_RD_DATA) & BIT_RD_BUSY)
    ;

  return MmioRead32 (ADI_BASE + REG_ADI_RD_DATA);
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
  MmioWrite32 (ADI_BASE + ADI_CHANNEL_OFFSET + Reg, Value);
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