/** @file
  SprdMusbDxe.c

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Protocol/SprdUsbDeviceController.h>
#include <Protocol/SprdUsbInit.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <SprdUsb.h>

#ifndef MUSB_CSR0_P_DATAEND
#define MUSB_CSR0_P_DATAEND       BIT3
#define MUSB_CSR0_P_SETUPEND     BIT4
#define MUSB_CSR0_P_SENDSTALL    BIT5
#define MUSB_CSR0_P_SVDRXPKTRDY  BIT6
#define MUSB_CSR0_P_SVDSETUPEND  BIT7
#define MUSB_TXCSR_P_SENDSTALL   BIT4
#define MUSB_TXCSR_P_SENTSTALL   BIT5
#define MUSB_TXCSR_P_WZC         (BIT7 | BIT5 | BIT2 | BIT1)
#define MUSB_RXCSR_P_SENDSTALL   BIT5
#define MUSB_RXCSR_P_WZC         (BIT6 | BIT2 | BIT0)
#endif
#define MUSB_EP_TXMAXP(Ep)       (((Ep) == 0U) ? 0x10U : (0x100U + ((Ep) * 0x10U)))
#define MUSB_EP_TXCSR(Ep)        (((Ep) == 0U) ? 0x12U : (0x102U + ((Ep) * 0x10U)))
#define MUSB_EP_RXMAXP(Ep)       (((Ep) == 0U) ? 0x14U : (0x104U + ((Ep) * 0x10U)))
#define MUSB_EP_RXCSR(Ep)        (((Ep) == 0U) ? 0x16U : (0x106U + ((Ep) * 0x10U)))
#define MUSB_EP_RXCOUNT(Ep)      (((Ep) == 0U) ? 0x18U : (0x108U + ((Ep) * 0x10U)))
#define MUSB_INTR_SUSPEND        BIT0
#define MUSB_INTR_RESUME         BIT1
#define MUSB_INTR_RESET          BIT2

#define SPRD_MUSB_SIGNATURE       SIGNATURE_32 ('S', 'M', 'U', 'B')
#define SPRD_MUSB_FROM_PROTOCOL(a) CR ((a), SPRD_MUSB, Controller, SPRD_MUSB_SIGNATURE)
#define SPRD_MUSB_EP_IN            5U
#define SPRD_MUSB_EP_OUT           6U
#define SPRD_MUSB_DMA_RX_CHANNEL   (SPRD_MUSB_EP_OUT + 15U)
#define SPRD_MUSB_DMA_BUFFER_SIZE  0xFFC0U
#define SPRD_MUSB_DMA_LIST_SIZE    64U
#define SPRD_MUSB_POLL_100NS       (1ULL * 1000ULL * 10ULL)

typedef struct {
  UINT32 Signature;
  SPRD_USB_DEVICE_CONTROLLER_PROTOCOL Controller;
  SPRD_USB_INIT_PROTOCOL *Init;
  EFI_EVENT PollEvent;
  EFI_EVENT ExitBootServicesEvent;
  SPRD_USB_DEVICE_RESET_CALLBACK ResetCallback;
  SPRD_USB_DEVICE_SETUP_CALLBACK SetupCallback;
  SPRD_USB_DEVICE_BULK_OUT_CALLBACK BulkOutCallback;
  VOID *Context;
  VOID *BulkContext;
  BOOLEAN Active;
  BOOLEAN BulkConfigured;
  UINT8 Address;
  UINT8 PendingAddress;
  BOOLEAN AddressPending;
  UINT8 BulkIn;
  UINT8 BulkOut;
  UINT16 BulkMps;
  UINT16 ActiveMps;
  EFI_PHYSICAL_ADDRESS DmaRxBuffer;
  EFI_PHYSICAL_ADDRESS DmaRxList;
  EFI_PHYSICAL_ADDRESS DmaRxBufferPhysical;
  EFI_PHYSICAL_ADDRESS DmaRxListPhysical;
  BOOLEAN DmaRxReady;
  BOOLEAN DmaRxArmed;
} SPRD_MUSB;

typedef struct {
  UINT32 Address;
  UINT16 FragmentLength;
  UINT16 BlockLength;
  UINT32 Flags;
  UINT32 Reserved;
} SPRD_MUSB_DMA_NODE;

STATIC UINT8 R8 (UINTN O) { return MmioRead8 (SPRD_MUSB_BASE + O); }
STATIC UINT16 R16 (UINTN O) { return MmioRead16 (SPRD_MUSB_BASE + O); }
STATIC UINT32 R32 (UINTN O) { return MmioRead32 (SPRD_MUSB_BASE + O); }
STATIC VOID W8 (UINTN O, UINT8 V) { MmioWrite8 (SPRD_MUSB_BASE + O, V); }
STATIC VOID W16 (UINTN O, UINT16 V) { MmioWrite16 (SPRD_MUSB_BASE + O, V); }
STATIC VOID W32 (UINTN O, UINT32 V) { MmioWrite32 (SPRD_MUSB_BASE + O, V); }
STATIC VOID SelectEp (UINT8 Ep) { W8 (MUSB_INDEX, Ep); }

STATIC
VOID
DmaRxClearStatus (
  VOID
  )
{
  W32 (MUSB_DMA_CHN_INTR (SPRD_MUSB_DMA_RX_CHANNEL),
    MUSB_DMA_LLIST_INT_CLEAR |
    MUSB_DMA_START_INT_CLEAR |
    MUSB_DMA_FRAG_INT_CLEAR |
    MUSB_DMA_BLOCK_INT_CLEAR |
    MUSB_DMA_USBRX_LAST_INT_CLEAR);
}

STATIC
EFI_STATUS
DmaRxAllocate (
  IN OUT SPRD_MUSB *Musb
  )
{
  EFI_PHYSICAL_ADDRESS Address;
  EFI_STATUS Status;
  UINTN Pages;

  if (Musb->DmaRxReady) {
    return EFI_SUCCESS;
  }
  Pages = EFI_SIZE_TO_PAGES (SPRD_MUSB_DMA_BUFFER_SIZE);
  Address = 0xFFFFFFFFULL;
  Status = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, Pages, &Address);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Musb->DmaRxBuffer = Address;

  Address = 0xFFFFFFFFULL;
  Status = gBS->AllocatePages (AllocateMaxAddress, EfiBootServicesData, 1, &Address);
  if (EFI_ERROR (Status)) {
    gBS->FreePages (Musb->DmaRxBuffer, Pages);
    Musb->DmaRxBuffer = 0;
    return Status;
  }
  Musb->DmaRxList = Address;
  Musb->DmaRxBufferPhysical = Musb->DmaRxBuffer;
  Musb->DmaRxListPhysical = Musb->DmaRxList;
  SetMem ((VOID *)(UINTN)Musb->DmaRxBuffer, SPRD_MUSB_DMA_BUFFER_SIZE, 0);
  SetMem ((VOID *)(UINTN)Musb->DmaRxList, EFI_PAGE_SIZE, 0);
  Musb->DmaRxReady = TRUE;
  return EFI_SUCCESS;
}

STATIC
VOID
DmaRxArm (
  IN OUT SPRD_MUSB *Musb
  )
{
  SPRD_MUSB_DMA_NODE *Node;

  if (!Musb->DmaRxReady) {
    return;
  }
  Node = (SPRD_MUSB_DMA_NODE *)(UINTN)Musb->DmaRxList;
  SetMem (Node, SPRD_MUSB_DMA_LIST_SIZE, 0);
  //
  // The node is consumed by the MUSB DMA engine, so it must contain the
  // DMA/physical address, not a CPU virtual address.
  //
  Node->Address = (UINT32)Musb->DmaRxBufferPhysical;
  Node->FragmentLength = 32;
  Node->BlockLength = (UINT16)SPRD_MUSB_DMA_BUFFER_SIZE;
  Node->Flags = BIT0 | BIT2; /* list_end + interrupt-on-completion */
  WriteBackInvalidateDataCacheRange (Node, SPRD_MUSB_DMA_LIST_SIZE);
  WriteBackInvalidateDataCacheRange (
    (VOID *)(UINTN)Musb->DmaRxBuffer, SPRD_MUSB_DMA_BUFFER_SIZE);

  W32 (MUSB_DMA_CHN_CFG (SPRD_MUSB_DMA_RX_CHANNEL), 0);
  DmaRxClearStatus ();
  W32 (MUSB_DMA_CHN_INTR (SPRD_MUSB_DMA_RX_CHANNEL),
    R32 (MUSB_DMA_CHN_INTR (SPRD_MUSB_DMA_RX_CHANNEL)) |
    MUSB_DMA_LLIST_INT_ENABLE | MUSB_DMA_USBRX_INT_ENABLE);
  W32 (MUSB_DMA_CHN_INTR (SPRD_MUSB_DMA_RX_CHANNEL),
    R32 (MUSB_DMA_CHN_INTR (SPRD_MUSB_DMA_RX_CHANNEL)) |
    MUSB_DMA_CLEAR_INT_ENABLE);
  W32 (MUSB_DMA_CHN_PAUSE (SPRD_MUSB_DMA_RX_CHANNEL), 0);
  W32 (MUSB_DMA_CHN_LLIST_PTR (SPRD_MUSB_DMA_RX_CHANNEL),
    (UINT32)Musb->DmaRxListPhysical);
  W32 (MUSB_DMA_CHN_CFG (SPRD_MUSB_DMA_RX_CHANNEL),
    R32 (MUSB_DMA_CHN_CFG (SPRD_MUSB_DMA_RX_CHANNEL)) |
    MUSB_DMA_CHN_ENABLE);
  Musb->DmaRxArmed = TRUE;
}

STATIC
VOID
FifoRead (
  IN UINT8 Ep,
  OUT UINT8 *Data,
  IN UINTN Length
  )
{
  UINTN  Fifo;
  UINT32 Value32;
  UINT16 Value16;

  //
  // MUSB FIFO is a fixed-width port.  The U-Boot MUSB driver uses the
  // widest access possible; byte-at-a-time reads are not reliable on this
  // controller and can return zeroes for a received packet.
  //
  Fifo = SPRD_MUSB_BASE + MUSB_FIFO_OFFSET (Ep);
  while (Length >= sizeof (UINT32)) {
    Value32 = MmioRead32 (Fifo);
    CopyMem (Data, &Value32, sizeof (Value32));
    Data += sizeof (Value32);
    Length -= sizeof (Value32);
  }
  if (Length >= sizeof (UINT16)) {
    Value16 = MmioRead16 (Fifo);
    CopyMem (Data, &Value16, sizeof (Value16));
    Data += sizeof (Value16);
    Length -= sizeof (Value16);
  }
  if (Length != 0) {
    *Data = MmioRead8 (Fifo);
  }
}

STATIC
VOID
FifoWrite (
  IN UINT8 Ep,
  IN CONST UINT8 *Data,
  IN UINTN Length
  )
{
  UINTN  Fifo;
  UINT32 Value32;
  UINT16 Value16;

  Fifo = SPRD_MUSB_BASE + MUSB_FIFO_OFFSET (Ep);
  while (Length >= sizeof (UINT32)) {
    CopyMem (&Value32, Data, sizeof (Value32));
    MmioWrite32 (Fifo, Value32);
    Data += sizeof (Value32);
    Length -= sizeof (Value32);
  }
  if (Length >= sizeof (UINT16)) {
    CopyMem (&Value16, Data, sizeof (Value16));
    MmioWrite16 (Fifo, Value16);
    Data += sizeof (Value16);
    Length -= sizeof (Value16);
  }
  if (Length != 0) {
    MmioWrite8 (Fifo, *Data);
  }
}

STATIC
VOID
ConfigureBulkHardware (
  IN SPRD_MUSB *Musb
  )
{
  UINT16 Packet;

  if (!Musb->BulkConfigured) {
    return;
  }
  Packet = ((R8 (MUSB_POWER) & MUSB_POWER_HSMODE) != 0) ? Musb->BulkMps : 64;
  Musb->ActiveMps = Packet;

  SelectEp (Musb->BulkIn);
  W8 (MUSB_TXFIFOSZ, 0x17);
  W16 (MUSB_TXFIFOADD, 0x108);
  W16 (MUSB_EP_TXMAXP (Musb->BulkIn), Packet);
  W8 (MUSB_EP_TXCSR (Musb->BulkIn), 0x48);
  W8 (MUSB_EP_TXCSRH (Musb->BulkIn), 0xB4);

  SelectEp (Musb->BulkOut);
  W8 (MUSB_RXFIFOSZ, R8 (MUSB_RXFIFOSZ) & (UINT8)~MUSB_FIFOSZ_DPB);
  W8 (MUSB_RXFIFOSZ, 0x17);
  W16 (MUSB_RXFIFOADD, 0x08);
  W16 (MUSB_EP_RXMAXP (Musb->BulkOut), Packet);
  W8 (MUSB_EP_RXCSR (Musb->BulkOut), 0x90);
  W8 (MUSB_EP_RXCSRH (Musb->BulkOut), 0xA8);

  W16 (MUSB_INTRTXE, BIT0 | (UINT16)(1U << Musb->BulkIn));
  W16 (MUSB_INTRRXE, (UINT16)(1U << Musb->BulkOut));
  if (!EFI_ERROR (DmaRxAllocate (Musb))) {
    DmaRxArm (Musb);
  } else {
    //
    // PIO fallback must not leave the endpoint in DMA mode.
    //
    SelectEp (Musb->BulkOut);
    W8 (MUSB_EP_RXCSRH (Musb->BulkOut), 0);
  }
  DEBUG ((DEBUG_INFO, "SPRD-MUSB: bulk EP%u IN/EP%u OUT MPS=%u speed=%a\n",
    Musb->BulkIn, Musb->BulkOut, Packet,
    ((R8 (MUSB_POWER) & MUSB_POWER_HSMODE) != 0) ? "HS" : "FS"));
}

STATIC
VOID
PollBulkOut (
  IN SPRD_MUSB *Musb
  )
{
  UINT16 Csr;
  UINT16 Count;
  UINT8 Buffer[512];

  if (!Musb->BulkConfigured || Musb->BulkOutCallback == NULL) {
    return;
  }
  if (Musb->DmaRxReady && Musb->DmaRxArmed) {
    UINT32 DmaIntr;
    UINT32 DmaLen;
    UINTN Received;
    EFI_STATUS Status;

    DmaIntr = R32 (MUSB_DMA_CHN_INTR (SPRD_MUSB_DMA_RX_CHANNEL));
    if ((DmaIntr & MUSB_DMA_LLIST_INT_STATUS) != 0) {
      DmaLen = R32 (MUSB_DMA_CHN_LEN (SPRD_MUSB_DMA_RX_CHANNEL));
      Received = (UINTN)SPRD_MUSB_DMA_BUFFER_SIZE -
        ((DmaLen >> 16) & 0xFFFFU);
      Musb->DmaRxArmed = FALSE;
      InvalidateDataCacheRange (
        (VOID *)(UINTN)Musb->DmaRxBuffer, SPRD_MUSB_DMA_BUFFER_SIZE);
      if (Received != 0 && Received <= SPRD_MUSB_DMA_BUFFER_SIZE) {
        Status = Musb->BulkOutCallback (
          Musb->BulkContext, Musb->BulkOut,
          (UINT8 *)(UINTN)Musb->DmaRxBuffer, Received);
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_WARN, "SPRD-MUSB: DMA RX callback failed: %r\n", Status));
        }
      }
      DmaRxArm (Musb);
    }
    return;
  }
  SelectEp (Musb->BulkOut);
  Csr = R16 (MUSB_EP_RXCSR (Musb->BulkOut));
  if ((Csr & MUSB_RXCSR_RXPKTRDY) == 0) {
    return;
  }
  Count = R16 (MUSB_EP_RXCOUNT (Musb->BulkOut));
  if (Count > sizeof (Buffer)) {
    while (Count-- != 0) {
      (VOID)MmioRead8 (SPRD_MUSB_BASE + MUSB_FIFO_OFFSET (Musb->BulkOut));
    }
    Csr |= MUSB_RXCSR_P_WZC;
    Csr &= (UINT16)~MUSB_RXCSR_RXPKTRDY;
    W16 (MUSB_EP_RXCSR (Musb->BulkOut), Csr);
    return;
  }
  FifoRead (Musb->BulkOut, Buffer, Count);
  Csr |= MUSB_RXCSR_P_WZC;
  Csr &= (UINT16)~MUSB_RXCSR_RXPKTRDY;
  W16 (MUSB_EP_RXCSR (Musb->BulkOut), Csr);
  {
    EFI_STATUS Status;
    Status = Musb->BulkOutCallback (Musb->BulkContext, Musb->BulkOut, Buffer, Count);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "SPRD-MUSB: PIO RX callback failed: %r\n", Status));
    }
  }
}

STATIC
VOID
PollDevice (
  IN SPRD_MUSB *Musb
  )
{
  UINT8 UsbInt;
  UINT16 Csr;
  UINT16 Count;
  UINT8 Setup[8];

  if (!Musb->Active) {
    return;
  }
  UsbInt = R8 (MUSB_INTRUSB);
  (VOID)R16 (MUSB_INTRTX);
  (VOID)R16 (MUSB_INTRRX);

  if ((UsbInt & MUSB_INTR_RESET) != 0) {
    Musb->Address = 0;
    Musb->PendingAddress = 0;
    Musb->AddressPending = FALSE;
    W8 (MUSB_FADDR, 0);
    SelectEp (0);
    W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_FLUSHFIFO);
    ConfigureBulkHardware (Musb);
    if (Musb->ResetCallback != NULL) {
      Musb->ResetCallback (Musb->Context);
    }
  }
  if ((UsbInt & MUSB_INTR_DISCONNECT) != 0) {
    DEBUG ((DEBUG_WARN, "SPRD-MUSB: USB DISCONNECT power=0x%02x addr=%u\n",
      R8 (MUSB_POWER), Musb->Address));
  }
  if ((UsbInt & MUSB_INTR_SUSPEND) != 0) {
    DEBUG ((DEBUG_VERBOSE, "SPRD-MUSB: USB SUSPEND\n"));
  }
  if ((UsbInt & MUSB_INTR_RESUME) != 0) {
    DEBUG ((DEBUG_VERBOSE, "SPRD-MUSB: USB RESUME\n"));
  }

  SelectEp (0);
  Csr = R16 (MUSB_EP_TXCSR (0));
  if ((Csr & MUSB_CSR0_P_SETUPEND) != 0) {
    W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_P_SVDSETUPEND);
    DEBUG ((DEBUG_WARN, "SPRD-MUSB: EP0 SETUPEND csr=0x%04x\n", Csr));
  }
  if (Musb->AddressPending && ((Csr & MUSB_CSR0_TXPKTRDY) == 0)) {
    W8 (MUSB_FADDR, Musb->PendingAddress);
    Musb->Address = Musb->PendingAddress;
    Musb->AddressPending = FALSE;
  }
  if ((Csr & MUSB_CSR0_RXPKTRDY) != 0) {
    Count = R16 (MUSB_EP_RXCOUNT (0));
    if (Count == sizeof (Setup)) {
      FifoRead (0, Setup, sizeof (Setup));
      W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_P_SVDRXPKTRDY);
      if (Musb->SetupCallback != NULL) {
        Musb->SetupCallback (Musb->Context, Setup);
      }
    } else {
      while (Count-- != 0) {
        (VOID)MmioRead8 (SPRD_MUSB_BASE + MUSB_FIFO_OFFSET (0));
      }
      W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_P_SENDSTALL);
    }
  }
  PollBulkOut (Musb);
}

STATIC
EFI_STATUS
EFIAPI
MusbStart (
  IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This,
  IN SPRD_USB_DEVICE_RESET_CALLBACK ResetCallback,
  IN SPRD_USB_DEVICE_SETUP_CALLBACK SetupCallback,
  IN VOID *Context
  )
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);

  if (ResetCallback == NULL || SetupCallback == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Musb->ResetCallback = ResetCallback;
  Musb->SetupCallback = SetupCallback;
  Musb->Context = Context;
  Musb->Active = TRUE;
  W8 (MUSB_POWER, 0);
  W8 (MUSB_INTRUSBE, 0);
  W16 (MUSB_INTRTXE, 0);
  W16 (MUSB_INTRRXE, 0);
  W8 (MUSB_TESTMODE, 0);
  W8 (MUSB_FADDR, 0);
  W8 (MUSB_DEVCTL, 0);
  (VOID)R8 (MUSB_INTRUSB);
  (VOID)R16 (MUSB_INTRTX);
  (VOID)R16 (MUSB_INTRRX);
  SelectEp (0);
  W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_FLUSHFIFO);
  W16 (MUSB_EP_TXCSR (0), 0);
  W8 (MUSB_INTRUSBE, MUSB_INTRUSBE_HOST_MASK);
  W16 (MUSB_INTRTXE, BIT0);
  W16 (MUSB_INTRRXE, 0);
  W8 (MUSB_POWER, MUSB_POWER_HSENAB | MUSB_POWER_SOFTCONN);
  DEBUG ((DEBUG_INFO, "SPRD-MUSB: device started POWER=0x%02x\n",
    R8 (MUSB_POWER)));
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbStop (IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This)
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  UINTN Pages;
  Musb->Active = FALSE;
  Musb->DmaRxArmed = FALSE;
  W32 (MUSB_DMA_CHN_CFG (SPRD_MUSB_DMA_RX_CHANNEL), 0);
  DmaRxClearStatus ();
  W8 (MUSB_POWER, R8 (MUSB_POWER) & (UINT8)~MUSB_POWER_SOFTCONN);
  W8 (MUSB_INTRUSBE, 0);
  W16 (MUSB_INTRTXE, 0);
  W16 (MUSB_INTRRXE, 0);
  if (Musb->DmaRxReady) {
    Pages = EFI_SIZE_TO_PAGES (SPRD_MUSB_DMA_BUFFER_SIZE);
    gBS->FreePages (Musb->DmaRxBuffer, Pages);
    gBS->FreePages (Musb->DmaRxList, 1);
    Musb->DmaRxBuffer = 0;
    Musb->DmaRxList = 0;
    Musb->DmaRxBufferPhysical = 0;
    Musb->DmaRxListPhysical = 0;
    Musb->DmaRxReady = FALSE;
  }
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbEp0Transmit (
  IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This, IN CONST UINT8 *Data,
  IN UINTN Length, IN UINT16 RequestedLength
  )
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  if (!Musb->Active || (Data == NULL && Length != 0) || Length > 64) {
    return EFI_INVALID_PARAMETER;
  }
  if (Length > RequestedLength) {
    Length = RequestedLength;
  }
  SelectEp (0);
  W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_P_SVDRXPKTRDY);
  FifoWrite (0, Data, Length);
  W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_TXPKTRDY | MUSB_CSR0_P_DATAEND);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbEp0Status (IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This)
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  if (!Musb->Active) return EFI_NOT_READY;
  SelectEp (0);
  W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_P_DATAEND);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbEp0Stall (IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This)
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  if (!Musb->Active) return EFI_NOT_READY;
  SelectEp (0);
  W16 (MUSB_EP_TXCSR (0), MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_P_SENDSTALL);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbSetAddress (
  IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This, IN UINT8 Address
  )
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  if (Address > 127 || !Musb->Active) return EFI_INVALID_PARAMETER;
  Musb->PendingAddress = Address;
  Musb->AddressPending = TRUE;
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbConfigureBulk (
  IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This, IN UINT8 InEp, IN UINT8 OutEp,
  IN UINT16 Mps, IN SPRD_USB_DEVICE_BULK_OUT_CALLBACK Callback, IN VOID *Context
  )
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  if (InEp != 0x85 || OutEp != 0x06 || Mps == 0 || Mps > 512 || Callback == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Musb->BulkIn = SPRD_MUSB_EP_IN;
  Musb->BulkOut = SPRD_MUSB_EP_OUT;
  Musb->BulkMps = Mps;
  Musb->BulkOutCallback = Callback;
  Musb->BulkContext = Context;
  Musb->BulkConfigured = TRUE;
  ConfigureBulkHardware (Musb);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbBulkTransmit (
  IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This, IN UINT8 Endpoint,
  IN CONST UINT8 *Data, IN UINTN Length, IN BOOLEAN Zlp
  )
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  UINTN Chunk;
  UINT16 Csr;
  UINTN Wait;
  if (!Musb->Active || !Musb->BulkConfigured || Endpoint != 0x85 ||
      (Data == NULL && Length != 0)) return EFI_NOT_READY;
  if (Length == 0) {
    if (!Zlp) return EFI_SUCCESS;
    SelectEp (Musb->BulkIn);
    for (Wait = 0; Wait < 300000; Wait++) {
      if ((R16 (MUSB_EP_TXCSR (Musb->BulkIn)) & MUSB_TXCSR_TXPKTRDY) == 0) break;
      MicroSecondDelay (10);
    }
    if (Wait == 300000) return EFI_TIMEOUT;
    W16 (MUSB_EP_TXCSR (Musb->BulkIn), MUSB_TXCSR_MODE | MUSB_TXCSR_TXPKTRDY);
    return EFI_SUCCESS;
  }
  do {
    SelectEp (Musb->BulkIn);
    for (Wait = 0; Wait < 300000; Wait++) {
      if ((R16 (MUSB_EP_TXCSR (Musb->BulkIn)) & MUSB_TXCSR_TXPKTRDY) == 0) break;
      MicroSecondDelay (10);
    }
    if (Wait == 300000) return EFI_TIMEOUT;
    Chunk = (Length > Musb->ActiveMps) ? Musb->ActiveMps : Length;
    FifoWrite (Musb->BulkIn, Data, Chunk);
    Csr = R16 (MUSB_EP_TXCSR (Musb->BulkIn));
    Csr &= (UINT16)~MUSB_TXCSR_P_UNDERRUN;
    W16 (MUSB_EP_TXCSR (Musb->BulkIn), Csr | MUSB_TXCSR_MODE | MUSB_TXCSR_TXPKTRDY);
    if (Data != NULL) Data += Chunk;
    Length -= Chunk;
  } while (Length != 0);
  if (Zlp) {
    SelectEp (Musb->BulkIn);
    for (Wait = 0; Wait < 300000; Wait++) {
      if ((R16 (MUSB_EP_TXCSR (Musb->BulkIn)) & MUSB_TXCSR_TXPKTRDY) == 0) break;
      MicroSecondDelay (10);
    }
    if (Wait == 300000) return EFI_TIMEOUT;
    W16 (MUSB_EP_TXCSR (Musb->BulkIn), MUSB_TXCSR_MODE | MUSB_TXCSR_TXPKTRDY);
  }
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbBulkReset (IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This)
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  ConfigureBulkHardware (Musb);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbBulkStall (
  IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This, IN UINT8 Endpoint
  )
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  UINT16 Csr;
  if (Endpoint == 0x85) {
    SelectEp (Musb->BulkIn); Csr = R16 (MUSB_EP_TXCSR (Musb->BulkIn));
    W16 (MUSB_EP_TXCSR (Musb->BulkIn), (Csr & (UINT16)~MUSB_TXCSR_P_WZC) | MUSB_TXCSR_P_SENDSTALL);
  } else if (Endpoint == 0x06) {
    SelectEp (Musb->BulkOut); Csr = R16 (MUSB_EP_RXCSR (Musb->BulkOut));
    W16 (MUSB_EP_RXCSR (Musb->BulkOut), (Csr & (UINT16)~MUSB_RXCSR_P_WZC) | MUSB_RXCSR_P_SENDSTALL);
  } else return EFI_INVALID_PARAMETER;
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI MusbReconnect (IN SPRD_USB_DEVICE_CONTROLLER_PROTOCOL *This)
{
  SPRD_MUSB *Musb = SPRD_MUSB_FROM_PROTOCOL (This);
  W8 (MUSB_POWER, R8 (MUSB_POWER) & (UINT8)~MUSB_POWER_SOFTCONN);
  MicroSecondDelay (300000);
  W8 (MUSB_POWER, R8 (MUSB_POWER) | MUSB_POWER_HSENAB | MUSB_POWER_SOFTCONN);
  return EFI_SUCCESS;
}

STATIC VOID EFIAPI MusbPollEvent (IN EFI_EVENT Event, IN VOID *Context)
{
  (VOID)Event;
  PollDevice (Context);
}

STATIC VOID EFIAPI MusbExitBootServices (IN EFI_EVENT Event, IN VOID *Context)
{
  (VOID)Event;
  SPRD_MUSB *Musb = (SPRD_MUSB *)Context;
  if (FixedPcdGetBool (PcdSprdKdNetDebug)) {
    (VOID)gBS->SetTimer (Musb->PollEvent, TimerCancel, 0);
    Musb->Active = FALSE;
    Musb->DmaRxArmed = FALSE;
    W32 (MUSB_DMA_CHN_CFG (SPRD_MUSB_DMA_RX_CHANNEL), 0);
    DmaRxClearStatus ();
    W8 (MUSB_INTRUSBE, 0);
    W16 (MUSB_INTRTXE, 0);
    W16 (MUSB_INTRRXE, 0);
    DEBUG ((DEBUG_INFO, "SPRD-MUSB: KDNET handoff stopped polling/DMA; PHY and device connection retained\\n"));
    return;
  }
  (VOID)MusbStop (&Musb->Controller);
}

STATIC CONST SPRD_USB_DEVICE_CONTROLLER_PROTOCOL mController = {
  SPRD_USB_DEVICE_CONTROLLER_PROTOCOL_REVISION,
  MusbStart, MusbStop, MusbEp0Transmit, MusbEp0Status, MusbEp0Stall,
  MusbSetAddress, MusbConfigureBulk, MusbBulkTransmit, MusbBulkReset,
  MusbBulkStall, MusbReconnect
};

EFI_STATUS EFIAPI SprdMusbDxeEntryPoint (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  SPRD_MUSB *Musb;
  (VOID)SystemTable;

  Musb = AllocateZeroPool (sizeof (*Musb));
  if (Musb == NULL) return EFI_OUT_OF_RESOURCES;
  Musb->Signature = SPRD_MUSB_SIGNATURE;
  CopyMem (&Musb->Controller, &mController, sizeof (mController));
  Status = gBS->LocateProtocol (&gSprdUsbInitProtocolGuid, NULL, (VOID **)&Musb->Init);
  if (EFI_ERROR (Status)) { FreePool (Musb); return Status; }
  if (Musb->Init->InitializePeripheral != NULL) {
    Status = Musb->Init->InitializePeripheral (Musb->Init);
    if (EFI_ERROR (Status)) { FreePool (Musb); return Status; }
  }
  W8 (MUSB_POWER, 0);
  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
    MusbPollEvent, Musb, &Musb->PollEvent);
  if (EFI_ERROR (Status)) { FreePool (Musb); return Status; }
  Status = gBS->SetTimer (Musb->PollEvent, TimerPeriodic, SPRD_MUSB_POLL_100NS);
  if (EFI_ERROR (Status)) { gBS->CloseEvent (Musb->PollEvent); FreePool (Musb); return Status; }
  Status = gBS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_NOTIFY, MusbExitBootServices,
    Musb, &gEfiEventExitBootServicesGuid, &Musb->ExitBootServicesEvent);
  if (EFI_ERROR (Status)) { gBS->CloseEvent (Musb->PollEvent); FreePool (Musb); return Status; }
  Status = gBS->InstallProtocolInterface (&ImageHandle, &gSprdUsbDeviceControllerProtocolGuid,
    EFI_NATIVE_INTERFACE, &Musb->Controller);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Musb->PollEvent);
    gBS->CloseEvent (Musb->ExitBootServicesEvent);
    FreePool (Musb);
  }
  return Status;
}