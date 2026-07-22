/** @file
  USB CDC-EEM device-mode driver for the UMS512 sharkl5pro MUSB controller.

  The platform U-Boot downloader already brings this PHY up.  The register
  setup below is intentionally idempotent, so DXE can also run after a cold
  handoff.  Endpoint 5 is bulk IN and endpoint 6 is bulk OUT, matching the
  U-Boot downloader layout.
**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SprdUsbEem.h>

#define SPRD_USB_BASE                 0x5FFF0000U
#define SPRD_AON_APB_BASE             0x327D0000U

#define AON_APB_EB1                   (SPRD_AON_APB_BASE + 0x0004U)
#define AON_APB_RST1                  (SPRD_AON_APB_BASE + 0x0010U)
#define AON_APB_OTG_PHY_TEST          (SPRD_AON_APB_BASE + 0x0204U)
#define AON_APB_OTG_PHY_CTRL          (SPRD_AON_APB_BASE + 0x0208U)
#define SPRD_ANLG_PHY_G2_BASE         0x323B0000U
#define ANLG_USB20_UTMI_CTL1          (SPRD_ANLG_PHY_G2_BASE + 0x0058U)
#define AON_OTG_UTMI_EB               BIT8
#define AON_ANA_EB                    BIT12
#define AON_OTG_UTMI_RST              BIT8
#define AON_OTG_PHY_RST               BIT9
#define AON_VBUS_VALID_PHYREG         BIT24
#define AON_UTMI_WIDTH_16             BIT30
#define ANLG_UTMI_VBUS_VALID          BIT16
#define ANLG_UTMI_WIDTH_16            BIT28
#define MUSB_FADDR                    0x00
#define MUSB_POWER                    0x01
#define MUSB_INTRTX                   0x02
#define MUSB_INTRRX                   0x04
#define MUSB_INTRTXE                  0x06
#define MUSB_INTRRXE                  0x08
#define MUSB_INTRUSB                  0x0A
#define MUSB_INTRUSBE                 0x0B
#define MUSB_INDEX                    0x0E
#define MUSB_FIFO(Endpoint)           (0x20 + ((Endpoint) * 4))
#define MUSB_DEVCTL                   0x60
#define MUSB_TXFIFOSZ                 0x62
#define MUSB_RXFIFOSZ                 0x63
#define MUSB_TXFIFOADD                0x64
#define MUSB_RXFIFOADD                0x66
#define MUSB_TXMAXP(Endpoint)         (0x100 + ((Endpoint) * 0x10))
#define MUSB_TXCSR(Endpoint)          (0x102 + ((Endpoint) * 0x10))
#define MUSB_RXMAXP(Endpoint)         (0x104 + ((Endpoint) * 0x10))
#define MUSB_RXCSR(Endpoint)          (0x106 + ((Endpoint) * 0x10))
#define MUSB_RXCOUNT(Endpoint)        (0x108 + ((Endpoint) * 0x10))
#define MUSB_CSR0                     0x12
#define MUSB_COUNT0                   0x18

#define MUSB_POWER_SOFTCONN           BIT6
#define MUSB_POWER_HSENAB             BIT5
#define MUSB_POWER_HSMODE             BIT4
#define MUSB_INTR_RESET                BIT2
#define MUSB_INTR_SUSPEND              BIT0
#define MUSB_INTR_RESUME               BIT1
#define MUSB_CSR0_RXPKTRDY            BIT0
#define MUSB_CSR0_TXPKTRDY            BIT1
#define MUSB_CSR0_P_SENTSTALL         BIT2
#define MUSB_CSR0_P_DATAEND           BIT3
#define MUSB_CSR0_P_SETUPEND          BIT4
#define MUSB_CSR0_P_SENDSTALL         BIT5
#define MUSB_CSR0_P_SVDRXPKTRDY       BIT6
#define MUSB_CSR0_P_SVDSETUPEND       BIT7
#define MUSB_TXCSR_TXPKTRDY           BIT0
#define MUSB_TXCSR_FLUSHFIFO          BIT3
#define MUSB_TXCSR_CLRDATATOG         BIT6
#define MUSB_RXCSR_RXPKTRDY           BIT0
#define MUSB_RXCSR_FLUSHFIFO          BIT4
#define MUSB_RXCSR_CLRDATATOG         BIT7

#define USB_REQ_GET_STATUS             0x00
#define USB_REQ_CLEAR_FEATURE          0x01
#define USB_REQ_SET_FEATURE            0x03
#define USB_REQ_SET_ADDRESS            0x05
#define USB_REQ_GET_DESCRIPTOR         0x06
#define USB_REQ_GET_CONFIGURATION      0x08
#define USB_REQ_SET_CONFIGURATION      0x09
#define USB_DESC_DEVICE                0x01
#define USB_DESC_CONFIGURATION         0x02
#define USB_DESC_STRING                0x03
#define USB_REQ_TYPE_STANDARD          0x00
#define USB_REQ_TYPE_CLASS             0x20
#define USB_REQ_DIR_IN                 0x80

#define EEM_BULK_IN_EP                 5
#define EEM_BULK_OUT_EP                6
#define EEM_MAX_FRAME                  1536
#define EEM_MAX_USB_FRAME              (EEM_MAX_FRAME + 2)
#define EEM_HIGH_SPEED_MPS             512
#define EEM_FULL_SPEED_MPS             64

typedef struct {
  UINT8   RequestType;
  UINT8   Request;
  UINT16  Value;
  UINT16  Index;
  UINT16  Length;
} USB_SETUP_PACKET;

typedef struct {
  SPRD_USB_EEM_PROTOCOL  Eem;
  EFI_HANDLE             Handle;
  EFI_EVENT              PollEvent;
  BOOLEAN                HardwareStarted;
  BOOLEAN                Configured;
  BOOLEAN                HighSpeed;
  UINT16                 MaxPacket;
  UINT8                  Configuration;
  UINT8                  PendingAddress;
  BOOLEAN                AddressPending;
  UINT8                  TxBuffer[EEM_MAX_USB_FRAME];
  UINTN                  TxLength;
  UINTN                  TxOffset;
  UINT8                  RxUsbBuffer[EEM_MAX_USB_FRAME];
  UINTN                  RxUsbLength;
  UINT8                  RxFrame[EEM_MAX_FRAME];
  UINTN                  RxFrameLength;
  BOOLEAN                RxReady;
} SPRD_USB_DEVICE;

STATIC SPRD_USB_DEVICE  *mDevice;

STATIC CONST UINT8  mDeviceDescriptor[] = {
  0x12, USB_DESC_DEVICE, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
  0x82, 0x17, 0x10, 0xE0, 0x00, 0x01, 0x01, 0x02, 0x03, 0x01
};

// CDC EEM is a single communication-class interface with two bulk endpoints.
STATIC UINT8  mConfigurationDescriptor[] = {
  0x09, USB_DESC_CONFIGURATION, 0x20, 0x00, 0x01, 0x01, 0x00, 0xC0, 0x32,
  0x09, 0x04, 0x00, 0x00, 0x02, 0x02, 0x0C, 0x07, 0x00,
  0x07, 0x05, 0x85, 0x02, 0x00, 0x02, 0x00,
  0x07, 0x05, 0x06, 0x02, 0x00, 0x02, 0x00
};

STATIC CONST UINT8  mLanguageString[] = { 0x04, USB_DESC_STRING, 0x09, 0x04 };
STATIC CONST UINT8  mManufacturerString[] = {
  0x20, USB_DESC_STRING, 'S', 0, 'p', 0, 'r', 0, 'e', 0, 'a', 0,
  'd', 0, 't', 0, 'r', 0, 'u', 0, 'm', 0, ' ', 0, 'U', 0, 'E', 0, 'F', 0, 'I', 0
};
STATIC CONST UINT8  mProductString[] = {
  0x26, USB_DESC_STRING, 'U', 0, 'M', 0, 'S', 0, '5', 0, '1', 0, '2', 0,
  ' ', 0, 'U', 0, 'S', 0, 'B', 0, ' ', 0, 'C', 0, 'D', 0, 'C', 0, '-', 0, 'E', 0, 'E', 0, 'M', 0
};
STATIC CONST UINT8  mSerialString[] = {
  0x1A, USB_DESC_STRING, '5', 0, '1', 0, '2', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '1', 0, 'E', 0, 'E', 0, 'M', 0
};

STATIC
UINT8
UsbRead8 (
  IN UINTN  Offset
  )
{
  return MmioRead8 (SPRD_USB_BASE + Offset);
}

STATIC
VOID
UsbWrite8 (
  IN UINTN  Offset,
  IN UINT8  Value
  )
{
  MmioWrite8 (SPRD_USB_BASE + Offset, Value);
}

STATIC
VOID
UsbFifoWrite (
  IN UINT8        Endpoint,
  IN CONST UINT8  *Buffer,
  IN UINTN        Length
  )
{
  while (Length-- != 0) {
    UsbWrite8 (MUSB_FIFO (Endpoint), *Buffer++);
  }
}

STATIC
VOID
UsbFifoRead (
  IN UINT8   Endpoint,
  OUT UINT8  *Buffer,
  IN UINTN   Length
  )
{
  while (Length-- != 0) {
    *Buffer++ = UsbRead8 (MUSB_FIFO (Endpoint));
  }
}

STATIC
VOID
SprdUsbConfigureEndpoints (
  IN SPRD_USB_DEVICE  *Device
  )
{
  UINT16  Mps;

  Mps = Device->MaxPacket;
  UsbWrite8 (MUSB_INDEX, EEM_BULK_IN_EP);
  UsbWrite8 (MUSB_TXFIFOSZ, 0x06); // 512-byte FIFO
  MmioWrite16 (SPRD_USB_BASE + MUSB_TXFIFOADD, 0x108);
  MmioWrite16 (SPRD_USB_BASE + MUSB_TXMAXP (EEM_BULK_IN_EP), Mps);
  MmioWrite16 (SPRD_USB_BASE + MUSB_TXCSR (EEM_BULK_IN_EP), MUSB_TXCSR_FLUSHFIFO | MUSB_TXCSR_CLRDATATOG);
  MmioWrite16 (SPRD_USB_BASE + MUSB_TXCSR (EEM_BULK_IN_EP), 0);

  UsbWrite8 (MUSB_INDEX, EEM_BULK_OUT_EP);
  UsbWrite8 (MUSB_RXFIFOSZ, 0x06); // 512-byte FIFO
  MmioWrite16 (SPRD_USB_BASE + MUSB_RXFIFOADD, 0x08);
  MmioWrite16 (SPRD_USB_BASE + MUSB_RXMAXP (EEM_BULK_OUT_EP), Mps);
  MmioWrite16 (SPRD_USB_BASE + MUSB_RXCSR (EEM_BULK_OUT_EP), MUSB_RXCSR_FLUSHFIFO | MUSB_RXCSR_CLRDATATOG);
  MmioWrite16 (SPRD_USB_BASE + MUSB_RXCSR (EEM_BULK_OUT_EP), 0);

  MmioWrite16 (SPRD_USB_BASE + MUSB_INTRTXE, BIT0 | (BIT0 << EEM_BULK_IN_EP));
  MmioWrite16 (SPRD_USB_BASE + MUSB_INTRRXE, BIT0 << EEM_BULK_OUT_EP);
}

STATIC
VOID
SprdUsbCompleteEp0 (
  IN BOOLEAN  DataEnd
  )
{
  UINT8  Csr;

  UsbWrite8 (MUSB_INDEX, 0);
  Csr = MUSB_CSR0_P_SVDRXPKTRDY;
  if (DataEnd) {
    Csr |= MUSB_CSR0_P_DATAEND;
  }

  UsbWrite8 (MUSB_CSR0, Csr);
}

STATIC
VOID
SprdUsbEp0Send (
  IN CONST UINT8  *Data,
  IN UINTN        DataLength,
  IN UINT16       RequestedLength
  )
{
  UINTN  Length;

  UsbWrite8 (MUSB_INDEX, 0);
  Length = MIN (DataLength, RequestedLength);
  UsbFifoWrite (0, Data, Length);
  UsbWrite8 (MUSB_CSR0, MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_TXPKTRDY | MUSB_CSR0_P_DATAEND);
}

STATIC
VOID
SprdUsbReset (
  IN SPRD_USB_DEVICE  *Device
  )
{
  Device->Configured = FALSE;
  Device->Configuration = 0;
  Device->PendingAddress = 0;
  Device->AddressPending = FALSE;
  Device->TxLength = 0;
  Device->TxOffset = 0;
  Device->RxUsbLength = 0;
  Device->RxReady = FALSE;
  // This SoC's boot ROM and U-Boot use the HS-only downloader configuration.
  // Do not read POWER until the MUSB AHB clock is known to be running: on a
  // gated UMS512 MUSB block a read produces an external abort while writes are
  // silently dropped.
  Device->HighSpeed = TRUE;
  Device->MaxPacket = EEM_HIGH_SPEED_MPS;
  mConfigurationDescriptor[22] = (UINT8)Device->MaxPacket;
  mConfigurationDescriptor[23] = (UINT8)(Device->MaxPacket >> 8);
  mConfigurationDescriptor[29] = (UINT8)Device->MaxPacket;
  mConfigurationDescriptor[30] = (UINT8)(Device->MaxPacket >> 8);
  UsbWrite8 (MUSB_FADDR, 0);
  SprdUsbConfigureEndpoints (Device);
}

STATIC
VOID
SprdUsbHandleSetup (
  IN SPRD_USB_DEVICE  *Device
  )
{
  USB_SETUP_PACKET  Setup;
  CONST UINT8       *Descriptor;
  UINTN             DescriptorLength;
  UINT8             Status[2] = { 0, 0 };

  UsbWrite8 (MUSB_INDEX, 0);
  UsbFifoRead (0, (UINT8 *)&Setup, sizeof (Setup));
  Descriptor = NULL;
  DescriptorLength = 0;

  if ((Setup.RequestType & 0x60) == USB_REQ_TYPE_STANDARD) {
    switch (Setup.Request) {
      case USB_REQ_GET_DESCRIPTOR:
        switch (Setup.Value >> 8) {
          case USB_DESC_DEVICE:
            Descriptor = mDeviceDescriptor;
            DescriptorLength = sizeof (mDeviceDescriptor);
            break;
          case USB_DESC_CONFIGURATION:
            Descriptor = mConfigurationDescriptor;
            DescriptorLength = sizeof (mConfigurationDescriptor);
            break;
          case USB_DESC_STRING:
            switch ((UINT8)Setup.Value) {
              case 0: Descriptor = mLanguageString; DescriptorLength = sizeof (mLanguageString); break;
              case 1: Descriptor = mManufacturerString; DescriptorLength = sizeof (mManufacturerString); break;
              case 2: Descriptor = mProductString; DescriptorLength = sizeof (mProductString); break;
              case 3: Descriptor = mSerialString; DescriptorLength = sizeof (mSerialString); break;
              default: break;
            }
            break;
          default:
            break;
        }
        if (Descriptor != NULL) {
          SprdUsbEp0Send (Descriptor, DescriptorLength, Setup.Length);
        } else {
          UsbWrite8 (MUSB_CSR0, MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_P_SENDSTALL);
        }
        return;

      case USB_REQ_SET_ADDRESS:
        // FADDR becomes active only after the address-zero status packet.
        Device->PendingAddress = (UINT8)Setup.Value;
        Device->AddressPending = TRUE;
        SprdUsbCompleteEp0 (TRUE);
        return;

      case USB_REQ_SET_CONFIGURATION:
        Device->Configuration = (UINT8)Setup.Value;
        Device->Configured = (BOOLEAN)(Device->Configuration == 1);
        if (Device->Configured) {
          SprdUsbConfigureEndpoints (Device);
        }
        SprdUsbCompleteEp0 (TRUE);
        return;

      case USB_REQ_GET_CONFIGURATION:
        SprdUsbEp0Send (&Device->Configuration, sizeof (Device->Configuration), Setup.Length);
        return;

      case USB_REQ_GET_STATUS:
        SprdUsbEp0Send (Status, sizeof (Status), Setup.Length);
        return;

      case USB_REQ_CLEAR_FEATURE:
      case USB_REQ_SET_FEATURE:
        SprdUsbCompleteEp0 (TRUE);
        return;

      default:
        break;
    }
  }

  // CDC-EEM has no mandatory class request; acknowledge host probes.
  if ((Setup.RequestType & 0x60) == USB_REQ_TYPE_CLASS) {
    if ((Setup.RequestType & USB_REQ_DIR_IN) != 0) {
      SprdUsbEp0Send (Status, 0, Setup.Length);
    } else {
      SprdUsbCompleteEp0 (TRUE);
    }
    return;
  }

  UsbWrite8 (MUSB_CSR0, MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_P_SENDSTALL);
}

STATIC
VOID
SprdUsbTxKick (
  IN SPRD_USB_DEVICE  *Device
  )
{
  UINTN  Chunk;

  if ((Device->TxOffset >= Device->TxLength) ||
      ((MmioRead16 (SPRD_USB_BASE + MUSB_TXCSR (EEM_BULK_IN_EP)) & MUSB_TXCSR_TXPKTRDY) != 0)) {
    return;
  }

  Chunk = MIN (Device->MaxPacket, Device->TxLength - Device->TxOffset);
  UsbFifoWrite (EEM_BULK_IN_EP, Device->TxBuffer + Device->TxOffset, Chunk);
  Device->TxOffset += Chunk;
  MmioOr16 (SPRD_USB_BASE + MUSB_TXCSR (EEM_BULK_IN_EP), MUSB_TXCSR_TXPKTRDY);
}

STATIC
VOID
SprdUsbConsumeEem (
  IN SPRD_USB_DEVICE  *Device
  )
{
  UINT16  Header;
  UINTN   PayloadLength;

  while (Device->RxUsbLength >= 2) {
    Header = (UINT16)(Device->RxUsbBuffer[0] | (Device->RxUsbBuffer[1] << 8));
    if ((Header & BIT15) != 0) {
      // EEM command packets have no place in the data path. Drop their header.
      CopyMem (Device->RxUsbBuffer, Device->RxUsbBuffer + 2, Device->RxUsbLength - 2);
      Device->RxUsbLength -= 2;
      continue;
    }

    PayloadLength = Header & 0x3FFF;
    if ((PayloadLength > EEM_MAX_FRAME) || (Device->RxUsbLength < PayloadLength + 2)) {
      return;
    }

    if (!Device->RxReady) {
      CopyMem (Device->RxFrame, Device->RxUsbBuffer + 2, PayloadLength);
      Device->RxFrameLength = PayloadLength;
      Device->RxReady = TRUE;
    }

    CopyMem (Device->RxUsbBuffer, Device->RxUsbBuffer + PayloadLength + 2, Device->RxUsbLength - PayloadLength - 2);
    Device->RxUsbLength -= PayloadLength + 2;
  }
}

STATIC
VOID
SprdUsbHandleRx (
  IN SPRD_USB_DEVICE  *Device
  )
{
  UINT16  Count;

  Count = MmioRead16 (SPRD_USB_BASE + MUSB_RXCOUNT (EEM_BULK_OUT_EP));
  if ((Count > Device->MaxPacket) || (Count > EEM_MAX_USB_FRAME - Device->RxUsbLength)) {
    MmioOr16 (SPRD_USB_BASE + MUSB_RXCSR (EEM_BULK_OUT_EP), MUSB_RXCSR_FLUSHFIFO);
    Device->RxUsbLength = 0;
    return;
  }

  UsbFifoRead (EEM_BULK_OUT_EP, Device->RxUsbBuffer + Device->RxUsbLength, Count);
  Device->RxUsbLength += Count;
  MmioAnd16 (SPRD_USB_BASE + MUSB_RXCSR (EEM_BULK_OUT_EP), (UINT16)~MUSB_RXCSR_RXPKTRDY);
  SprdUsbConsumeEem (Device);
}

STATIC
VOID
SprdUsbInitializePhy (
  VOID
  )
{
  // This is the sharkl5pro usb_phy_init() sequence from the supplied U-Boot.
  // It is required after resets which skip the U-Boot downloader.
  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: enabling MUSB clock gate\n"));
  MmioOr32 (AON_APB_EB1, AON_OTG_UTMI_EB | AON_ANA_EB);

  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: forcing PHY VBUS valid\n"));
  MmioOr32 (AON_APB_OTG_PHY_TEST, AON_VBUS_VALID_PHYREG);
  MmioOr32 (ANLG_USB20_UTMI_CTL1, ANLG_UTMI_VBUS_VALID);

  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: selecting 16-bit UTMI\n"));
  MmioOr32 (AON_APB_OTG_PHY_CTRL, AON_UTMI_WIDTH_16);
  MmioOr32 (ANLG_USB20_UTMI_CTL1, ANLG_UTMI_WIDTH_16);

  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: resetting USB PHY\n"));
  MmioOr32 (AON_APB_RST1, AON_OTG_PHY_RST | AON_OTG_UTMI_RST);
  MicroSecondDelay (5000);
  MmioAnd32 (AON_APB_RST1, ~(AON_OTG_PHY_RST | AON_OTG_UTMI_RST));
  MicroSecondDelay (5000);
  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: USB PHY ready\n"));
}

STATIC
VOID
SprdUsbStartHardware (
  IN SPRD_USB_DEVICE  *Device
  )
{
  SprdUsbInitializePhy ();

  // Force a visible disconnect before taking over from U-Boot, then leave
  // DEVCTL in B-device/peripheral state.  Connecting before EP0/FIFOs are
  // ready can make a Windows host abandon its first reset attempt.
  UsbWrite8 (MUSB_POWER, 0);
  UsbWrite8 (MUSB_INTRUSBE, 0);
  MmioWrite16 (SPRD_USB_BASE + MUSB_INTRTXE, 0);
  MmioWrite16 (SPRD_USB_BASE + MUSB_INTRRXE, 0);
  UsbWrite8 (MUSB_DEVCTL, 0);
  MicroSecondDelay (20000);

  SprdUsbReset (Device);
  UsbWrite8 (MUSB_INTRUSBE, 0xF7);
  UsbWrite8 (MUSB_POWER, MUSB_POWER_HSENAB);
  MicroSecondDelay (10000);
  UsbWrite8 (MUSB_POWER, MUSB_POWER_SOFTCONN | MUSB_POWER_HSENAB);
  Device->HardwareStarted = TRUE;
  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: MUSB core started\n"));
}

STATIC
VOID
EFIAPI
SprdUsbPoll (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  SPRD_USB_DEVICE  *Device;
  UINT8            UsbInterrupt;
  UINT16           TxInterrupt;
  UINT16           RxInterrupt;

  Device = Context;
  if (!Device->HardwareStarted) {
    SprdUsbStartHardware (Device);
    return;
  }

  UsbInterrupt = UsbRead8 (MUSB_INTRUSB);
  if ((UsbInterrupt & MUSB_INTR_RESET) != 0) {
    SprdUsbReset (Device);
  }

  TxInterrupt = MmioRead16 (SPRD_USB_BASE + MUSB_INTRTX);
  if ((TxInterrupt & BIT0) != 0) {
    UsbWrite8 (MUSB_INDEX, 0);
    if ((UsbRead8 (MUSB_CSR0) & MUSB_CSR0_RXPKTRDY) != 0) {
      SprdUsbHandleSetup (Device);
    } else if (Device->AddressPending) {
      UsbWrite8 (MUSB_FADDR, Device->PendingAddress);
      Device->AddressPending = FALSE;
    }
  }

  if ((TxInterrupt & (BIT0 << EEM_BULK_IN_EP)) != 0) {
    SprdUsbTxKick (Device);
  }

  RxInterrupt = MmioRead16 (SPRD_USB_BASE + MUSB_INTRRX);
  if ((RxInterrupt & (BIT0 << EEM_BULK_OUT_EP)) != 0) {
    SprdUsbHandleRx (Device);
  }
}

STATIC
EFI_STATUS
EFIAPI
SprdUsbEemSendFrame (
  IN SPRD_USB_EEM_PROTOCOL  *This,
  IN CONST VOID             *Frame,
  IN UINTN                  FrameLength
  )
{
  SPRD_USB_DEVICE  *Device;
  UINT16           Header;

  if ((Frame == NULL) || (FrameLength == 0) || (FrameLength > EEM_MAX_FRAME)) {
    return EFI_INVALID_PARAMETER;
  }

  Device = BASE_CR (This, SPRD_USB_DEVICE, Eem);
  if (!Device->Configured) {
    return EFI_NOT_READY;
  }

  if (Device->TxOffset < Device->TxLength) {
    return EFI_NOT_READY;
  }

  Header = (UINT16)FrameLength;
  Device->TxBuffer[0] = (UINT8)Header;
  Device->TxBuffer[1] = (UINT8)(Header >> 8);
  CopyMem (Device->TxBuffer + 2, Frame, FrameLength);
  Device->TxLength = FrameLength + 2;
  Device->TxOffset = 0;
  SprdUsbTxKick (Device);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SprdUsbEemReceiveFrame (
  IN     SPRD_USB_EEM_PROTOCOL  *This,
  OUT    VOID                   *Frame,
  IN OUT UINTN                  *FrameLength
  )
{
  SPRD_USB_DEVICE  *Device;

  if ((Frame == NULL) || (FrameLength == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Device = BASE_CR (This, SPRD_USB_DEVICE, Eem);
  if (!Device->RxReady) {
    return EFI_NOT_READY;
  }

  if (*FrameLength < Device->RxFrameLength) {
    *FrameLength = Device->RxFrameLength;
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem (Frame, Device->RxFrame, Device->RxFrameLength);
  *FrameLength = Device->RxFrameLength;
  Device->RxReady = FALSE;
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
EFIAPI
SprdUsbEemLinkUp (
  IN SPRD_USB_EEM_PROTOCOL  *This
  )
{
  SPRD_USB_DEVICE  *Device;

  Device = BASE_CR (This, SPRD_USB_DEVICE, Eem);
  return Device->Configured;
}

EFI_STATUS
EFIAPI
SprdUsbDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  mDevice = AllocateZeroPool (sizeof (*mDevice));
  if (mDevice == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mDevice->Eem.SendFrame = SprdUsbEemSendFrame;
  mDevice->Eem.ReceiveFrame = SprdUsbEemReceiveFrame;
  mDevice->Eem.LinkUp = SprdUsbEemLinkUp;
  mDevice->MaxPacket = EEM_HIGH_SPEED_MPS;
  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: entry\n"));

  Status = gBS->InstallProtocolInterface (
                  &mDevice->Handle,
                  &gSprdUsbEemProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mDevice->Eem
                  );
  if (EFI_ERROR (Status)) {
    FreePool (mDevice);
    mDevice = NULL;
    return Status;
  }

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SprdUsbPoll,
                  mDevice,
                  &mDevice->PollEvent
                  );
  if (EFI_ERROR (Status)) {
    gBS->UninstallProtocolInterface (mDevice->Handle, &gSprdUsbEemProtocolGuid, &mDevice->Eem);
    FreePool (mDevice);
    mDevice = NULL;
    return Status;
  }

  Status = gBS->SetTimer (mDevice->PollEvent, TimerPeriodic, 10000); // 1 ms
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (mDevice->PollEvent);
    gBS->UninstallProtocolInterface (mDevice->Handle, &gSprdUsbEemProtocolGuid, &mDevice->Eem);
    FreePool (mDevice);
    mDevice = NULL;
    return Status;
  }

  DEBUG ((DEBUG_ERROR, "SprdUsbDxe: waiting to take over MUSB\n"));
  return Status;
}

EFI_STATUS
EFIAPI
SprdUsbDxeUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  if (mDevice == NULL) {
    return EFI_SUCCESS;
  }

  if (mDevice->HardwareStarted) {
    UsbWrite8 (MUSB_POWER, 0);
  }
  if (mDevice->PollEvent != NULL) {
    gBS->CloseEvent (mDevice->PollEvent);
  }

  gBS->UninstallProtocolInterface (mDevice->Handle, &gSprdUsbEemProtocolGuid, &mDevice->Eem);
  FreePool (mDevice);
  mDevice = NULL;
  return EFI_SUCCESS;
}
