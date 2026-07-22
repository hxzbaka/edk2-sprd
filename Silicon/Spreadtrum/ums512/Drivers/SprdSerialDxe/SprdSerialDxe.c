/** @file
  SprdSerialDxe - Install EFI_SERIAL_IO_PROTOCOL on SC9836 UART.
  UART hardware is assumed already initialized by U-Boot.
*/

#include <Uefi.h>
#include <Protocol/SerialIo.h>
#include <Protocol/DevicePath.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseLib.h>

//--------------------- UART Registers (from your existing driver) -----------------------
#define SPRD_UART_BASE    FixedPcdGet64(PcdDebugUartPortBase)   // Must be set in .dsc

#define SPRD_TXD          0x0000
#define SPRD_RXD          0x0004
#define SPRD_LSR          0x0008
#define SPRD_STS1         0x000C

#define SPRD_STS1_RX_CNT  0x00FF
#define SPRD_STS1_TX_CNT  0xFF00

// Helper registers for initialization (matching your SerialPortInitialize)
#define SPRD_CTL1         0x001C
#define SPRD_CTL2         0x0020
#define SPRD_CTL3         0x0010
#define SPRD_CTL4         0x0014

//--------------------- Local UART functions (same as your SerialPortLib) ----------------
STATIC
UINTN
UartWrite (
  IN UINT8 *Buffer,
  IN UINTN NumberOfBytes
  )
{
  UINTN i;
  for (i = 0; i < NumberOfBytes; i++) {
    if (Buffer[i] == '\n') {
      while (MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_TX_CNT);
      MmioWrite32 (SPRD_UART_BASE + SPRD_TXD, '\r');
    }
    while (MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_TX_CNT);
    MmioWrite32 (SPRD_UART_BASE + SPRD_TXD, Buffer[i]);
  }
  while (MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_TX_CNT);
  return NumberOfBytes;
}

STATIC
UINTN
UartRead (
  OUT UINT8 *Buffer,
  IN  UINTN NumberOfBytes
  )
{
  UINTN i;
  for (i = 0; i < NumberOfBytes; i++) {
    while (!(MmioRead32 (SPRD_UART_BASE + SPRD_STS1) & SPRD_STS1_RX_CNT));
    Buffer[i] = (UINT8)(MmioRead32 (SPRD_UART_BASE + SPRD_RXD) & 0xFF);
  }
  return NumberOfBytes;
}

//--------------------- SerialIo Protocol implementation --------------------------------
EFI_SERIAL_IO_MODE gSerialIoMode = {
  0,               // ControlMask
  1000000,         // Timeout (1 sec)
  115200,          // BaudRate
  16,              // ReceiveFifoDepth (typical)
  8,               // DataBits
  NoParity,        // Parity
  OneStopBit       // StopBits
};

STATIC
EFI_STATUS
EFIAPI
SerialIoReset (
  IN EFI_SERIAL_IO_PROTOCOL *This
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SerialIoSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT64                 BaudRate,
  IN UINT32                 ReceiveFifoDepth,
  IN UINT32                 Timeout,
  IN EFI_PARITY_TYPE        Parity,
  IN UINT8                  DataBits,
  IN EFI_STOP_BITS_TYPE     StopBits
  )
{
  // We ignore parameters; hardware is fixed.
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SerialIoSetControl (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT32                 Control
  )
{
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
SerialIoGetControl (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  OUT UINT32                *Control
  )
{
  *Control = 0;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SerialIoWrite (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN OUT UINTN              *BufferSize,
  IN VOID                   *Buffer
  )
{
  UartWrite (Buffer, *BufferSize);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SerialIoRead (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN OUT UINTN              *BufferSize,
  OUT VOID                  *Buffer
  )
{
  *BufferSize = UartRead (Buffer, *BufferSize);
  return EFI_SUCCESS;
}

EFI_SERIAL_IO_PROTOCOL gSerialIoProtocol = {
  SERIAL_IO_INTERFACE_REVISION,
  SerialIoReset,
  SerialIoSetAttributes,
  SerialIoSetControl,
  SerialIoGetControl,
  SerialIoWrite,
  SerialIoRead,
  &gSerialIoMode
};

//--------------------- Device Path -----------------------------------------------------
#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH        Vendor;
  UART_DEVICE_PATH          Uart;
  EFI_DEVICE_PATH_PROTOCOL  End;
} SPRD_SERIAL_DP;
#pragma pack()

STATIC SPRD_SERIAL_DP gDevicePath = {
  {
    { HARDWARE_DEVICE_PATH, HW_VENDOR_DP, { sizeof (VENDOR_DEVICE_PATH), 0 } },
    { 0xd3987b4b, 0x971f, 0x4994, { 0x87, 0x1c, 0x9f, 0x7b, 0x8c, 0x11, 0x86, 0x35 } }  // Random GUID
  },
  {
    { MESSAGING_DEVICE_PATH, MSG_UART_DP, { sizeof (UART_DEVICE_PATH), 0 } },
    0,                    // Reserved
    115200,               // BaudRate
    8,                    // DataBits
    (UINT8) NoParity,     // Parity
    (UINT8) OneStopBit    // StopBits
  },
  { END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, { sizeof (EFI_DEVICE_PATH_PROTOCOL), 0 } }
};

//--------------------- Driver Entry Point ----------------------------------------------
EFI_STATUS
EFIAPI
SprdSerialDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  // Install SerialIo and Device Path
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiSerialIoProtocolGuid,    &gSerialIoProtocol,
                  &gEfiDevicePathProtocolGuid,  &gDevicePath,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "SprdSerialDxe: SerialIo installed on UART base 0x%lx\n", SPRD_UART_BASE));
  return EFI_SUCCESS;
}