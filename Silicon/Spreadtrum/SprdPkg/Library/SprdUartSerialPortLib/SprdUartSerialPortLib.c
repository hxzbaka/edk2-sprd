/**
@file
SC9836-UART Drivers
Copyright (c) 2026, UNISOC WOA Project
Copyright (c) 2026, hxzbaka. 
**/

#include <Uefi.h>
#include <Library/SerialPortLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>

#define SPRD_UART_BASE                             FixedPcdGet64(PcdDebugUartPortBase)

/* BIT  */
#define SPRD_LSR_TX_OVER  (1UL << 15)
#define SPRD_STS1_RX_CNT  0x00FF
#define SPRD_STS1_TX_CNT  0xFF00
#define SPRD_TXD                     0x0000
#define SPRD_RXD                     0x0004
#define SPRD_LSR                      0x0008
#define SPRD_STS1                    0x000C

UINTN
EFIAPI
SerialPortWrite (
  IN UINT8 *Buffer,
  IN UINTN NumberOfBytes
  )
{
  UINTN Index;

  for (Index = 0; Index < NumberOfBytes; Index++) {
    if (Buffer[Index] == '\n') {
      while (MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_TX_CNT);
      MmioWrite32 (SPRD_UART_BASE + SPRD_TXD, '\r');
    }
    while (MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_TX_CNT);
    MmioWrite32 (SPRD_UART_BASE + SPRD_TXD, Buffer[Index]);
  }

  while (MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_TX_CNT);

  return NumberOfBytes;
}

UINTN
EFIAPI
SerialPortRead (
  OUT UINT8 *Buffer,
  IN  UINTN NumberOfBytes
  )
{
  UINTN Index;

  for (Index = 0; Index < NumberOfBytes; Index++) {
    while (!(MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_RX_CNT));
    
    Buffer[Index] = (UINT8)(MmioRead32 (SPRD_UART_BASE + SPRD_RXD) & 0xFF);
  }

  return NumberOfBytes;
}

BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return (BOOLEAN)((MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_RX_CNT) != 0);
}

RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINT32 Ctl1;
  CHAR8  Buffer[128];
  UINTN  CharCount;

  Ctl1 = MmioRead32 (SPRD_UART_BASE + 0x001C);
  Ctl1 &= ~((1UL << 7) | (1UL << 8));
  MmioWrite32 (SPRD_UART_BASE + 0x001C, Ctl1);
  MmioWrite32 (SPRD_UART_BASE + 0x0020, (1UL << 8) | 1UL);
  MmioWrite32 (SPRD_UART_BASE + 0x0010, 0);
  MmioWrite32 (SPRD_UART_BASE + 0x0014, 0xFFFFFFFF);


  return RETURN_SUCCESS;
}

RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64             *BaudRate,
  IN OUT UINT32             *ReceiveFifoDepth,
  IN OUT UINT32             *Timeout,
  IN OUT EFI_PARITY_TYPE    *Parity,
  IN OUT UINT8              *DataBits,
  IN OUT EFI_STOP_BITS_TYPE *StopBits
  )
{
  return RETURN_SUCCESS;
}

RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32 Control
  )
{
  return RETURN_SUCCESS;
}