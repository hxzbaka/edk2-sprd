#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SprdDisplayPanelLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define SPRD_GPIO_DATA      0x00
#define SPRD_GPIO_DMSK      0x04
#define SPRD_GPIO_DIR       0x08
#define SPRD_GPIO_INEN      0x28
#define SPRD_GPIO_BANK_SIZE 0x80
#define SPRD_GPIO_BANK_PINS 16U

STATIC CONST UINT8 PanelInitCommands[] = {
  0x39, 0x00, 0x00, 0x05, 0x2A, 0x00, 0x10, 0x01, 0x7F,
  0x39, 0x00, 0x00, 0x05, 0x2B, 0x00, 0x00, 0x01, 0xBF,
  0x15, 0x00, 0x00, 0x02, 0x35, 0x00,
  0x15, 0x00, 0x00, 0x02, 0x36, 0x00,
  0x15, 0x00, 0x00, 0x02, 0x3A, 0x07,
  0x15, 0x00, 0x00, 0x02, 0x64, 0x10,
  0x15, 0x00, 0x00, 0x02, 0x67, 0x01,
  0x15, 0x00, 0x00, 0x02, 0x68, 0x01,
  0x15, 0x00, 0x00, 0x02, 0xF0, 0x50,
  0x15, 0x00, 0x00, 0x02, 0xB9, 0x00,
  0x15, 0x0A, 0x00, 0x02, 0x51, 0x00,
  0x05, 0x78, 0x00, 0x01, 0x11,
  0x05, 0x14, 0x00, 0x01, 0x29,
  0x15, 0x0A, 0x00, 0x02, 0x51, 0xFF
};

STATIC
VOID
PanelGpioWrite(IN UINT32 Gpio, IN BOOLEAN High)
{
  UINTN  BankBase;
  UINT32 Mask;

  BankBase = FixedPcdGet32(PcdApGpioBaseAddress) +
      ((Gpio / SPRD_GPIO_BANK_PINS) * SPRD_GPIO_BANK_SIZE);
  Mask = 1U << (Gpio % SPRD_GPIO_BANK_PINS);

  MmioOr32(BankBase + SPRD_GPIO_DMSK, Mask);
  MmioOr32(BankBase + SPRD_GPIO_DIR, Mask);
  MmioAnd32(BankBase + SPRD_GPIO_INEN, ~Mask);
  if (High) {
    MmioOr32(BankBase + SPRD_GPIO_DATA, Mask);
  } else {
    MmioAnd32(BankBase + SPRD_GPIO_DATA, ~Mask);
  }
}

EFI_STATUS
EFIAPI
SprdDisplayPanelPrepare(IN CONST SPRD_DISPLAY_PANEL_IO *Io)
{
  (VOID)Io;
  if ((FixedPcdGet32(PcdApGpioBaseAddress) == 0) ||
      (FixedPcdGet32(PcdPanelAvddGpio) == 0) ||
      (FixedPcdGet32(PcdPanelResetGpio) == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  PanelGpioWrite(FixedPcdGet32(PcdPanelAvddGpio), TRUE);
  gBS->Stall(20000);
  PanelGpioWrite(FixedPcdGet32(PcdPanelResetGpio), TRUE);
  gBS->Stall(20000);
  PanelGpioWrite(FixedPcdGet32(PcdPanelResetGpio), FALSE);
  gBS->Stall(20000);
  PanelGpioWrite(FixedPcdGet32(PcdPanelResetGpio), TRUE);
  gBS->Stall(120000);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SprdDisplayPanelEnable(IN CONST SPRD_DISPLAY_PANEL_IO *Io)
{
  EFI_STATUS Status;
  UINTN      Offset;
  UINT16     PayloadLength;

  if ((Io == NULL) || (Io->Write == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Offset = 0;
  while (Offset < sizeof(PanelInitCommands)) {
    if ((sizeof(PanelInitCommands) - Offset) < 4) {
      return EFI_COMPROMISED_DATA;
    }

    PayloadLength = (UINT16)(PanelInitCommands[Offset + 2] << 8) |
        PanelInitCommands[Offset + 3];
    if (PayloadLength > (sizeof(PanelInitCommands) - Offset - 4)) {
      return EFI_COMPROMISED_DATA;
    }

    Status = Io->Write(
        Io->Context,
        &PanelInitCommands[Offset + 4],
        PayloadLength);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    if (PanelInitCommands[Offset + 1] != 0) {
      gBS->Stall((UINTN)PanelInitCommands[Offset + 1] * 1000);
    }

    Offset += PayloadLength + 4;
  }

  return EFI_SUCCESS;
}
