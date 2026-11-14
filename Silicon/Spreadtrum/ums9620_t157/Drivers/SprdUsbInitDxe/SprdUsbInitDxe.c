/** @file
  SprdUsbInitDxe.c

  UMS512 USB2 PHY and OTG host initialisation.
  U-Boot leaves this controller in gadget mode.  This driver deliberately
  disconnects gadget, resets the PHY, selects an A-device, and starts a new
  host session before MUSB is exposed to the UEFI USB bus driver.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <SprdUsb.h>
#include <Library/DebugLib.h>


#ifndef PcdSprdUsbFan54015Address
#define PcdSprdUsbFan54015Address  0x6A
#endif

#define FAN54015_REG_CONTROL1  0x01
#define FAN54015_REG_CONTROL0  0x00
#define FAN54015_OTG_ENABLE    BIT0
#define FAN54015_HZ_MODE       BIT1
#define FAN54015_RESET_WATCHDOG BIT7

/*
  The FAN54015 watchdog resets the charger/boost state unless REG00.bit7 is
  periodically written back as one.  Linux sends its first keep-alive 50 ms
  after enabling OTG and then services it every 15 seconds.  Use a shorter
  period here: UEFI has no charger framework and must retain VBUS itself.
*/
#define FAN54015_WATCHDOG_PERIOD_100NS  (5ULL * 1000ULL * 1000ULL * 10ULL)

STATIC EFI_EVENT  mFan54015WatchdogEvent;

STATIC
VOID
SprdMmioAndThenOr32 (
  IN UINTN  Address,
  IN UINT32 AndMask,
  IN UINT32 OrMask
  )
{
  MmioWrite32 (Address, (MmioRead32 (Address) & AndMask) | OrMask);
}

STATIC
UINT16
SprdAdiRead16 (
  IN UINT16  Register
  )
{
  UINTN   Retry;
  UINT32  Value;

  MmioWrite32 (SPRD_ADI_BASE + SPRD_ADI_RD_CMD, Register);
  for (Retry = 0; Retry < 2000; Retry++) {
    Value = MmioRead32 (SPRD_ADI_BASE + SPRD_ADI_RD_DATA);
    if ((Value & SPRD_ADI_RD_BUSY) == 0) {
      return (UINT16)Value;
    }
    MicroSecondDelay (1);
  }

  DEBUG ((DEBUG_ERROR, "SPRD-USB: ADI read timeout at 0x%04x\n", Register));
  return MAX_UINT16;
}

STATIC
BOOLEAN
SprdTypecForceSource (
  VOID
  )
{
  UINTN   Aon;
  UINTN   Retry;
  UINT16  Mode;
  UINT16  Enable;
  UINT16  InterruptEnable;
  UINT16  Status;

  Aon = SPRD_AON_APB_BASE;
  MmioOr32 (Aon + AON_APB_EB0, AON_ADI_EB);

  /*
    This board's USB-C receptacle is managed by SC2730 typec@380.  Its DTS
    selects DRP mode for Android, but UEFI is deliberately acting as a fixed
    OTG host.  Advertise Rp before enabling VBUS so a Type-C OTG hub exposes
    its USB2 data path to MUSB.
  */
  Mode = SprdAdiRead16 (SPRD_SC2730_TYPEC_REG + SC2730_TYPEC_MODE);
  Enable = SprdAdiRead16 (SPRD_SC2730_TYPEC_REG + SC2730_TYPEC_EN);
  InterruptEnable = SprdAdiRead16 (SPRD_SC2730_TYPEC_REG + SC2730_TYPEC_INT_EN);
  Status = SprdAdiRead16 (SPRD_SC2730_TYPEC_REG + SC2730_TYPEC_STATUS);
  if ((Mode == MAX_UINT16) || (Enable == MAX_UINT16) ||
      (InterruptEnable == MAX_UINT16) || (Status == MAX_UINT16)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: SC2730 Type-C setup skipped after ADI read failure\n"));
    return FALSE;
  }
  DEBUG ((DEBUG_INFO,
    "SPRD-USB: SC2730 Type-C before host force: MODE=0x%04x EN=0x%04x INT_EN=0x%04x STATUS=0x%04x\n",
    Mode, Enable, InterruptEnable, Status));

  Mode = (UINT16)((Mode & ~SC2730_TYPEC_MODE_MASK) | SC2730_TYPEC_MODE_SRC);
  Enable &= (UINT16)~SC2730_TYPEC_USB20_ONLY;
  InterruptEnable |= SC2730_TYPEC_ATTACH_INT_EN | SC2730_TYPEC_DETACH_INT_EN;
  MmioWrite32 (SPRD_SC2730_TYPEC_BASE + SC2730_TYPEC_MODE, Mode);
  MmioWrite32 (SPRD_SC2730_TYPEC_BASE + SC2730_TYPEC_EN, Enable);
  MmioWrite32 (SPRD_SC2730_TYPEC_BASE + SC2730_TYPEC_INT_CLR,
    SC2730_TYPEC_ATTACH_INT_EN | SC2730_TYPEC_DETACH_INT_EN);
  MmioWrite32 (SPRD_SC2730_TYPEC_BASE + SC2730_TYPEC_INT_EN, InterruptEnable);

  /*
    Type-C attaches only after CC debounce.  The 50 ms delay used for the
    FAN54015 boost is not sufficient on this board: the PMIC reports
    ATTACHWAIT_SRC first, then ATTACHED_SRC after the source debounce timer.
  */
  for (Retry = 0; Retry < 50; Retry++) {
    MicroSecondDelay (10000);
    Status = SprdAdiRead16 (SPRD_SC2730_TYPEC_REG + SC2730_TYPEC_STATUS);
    if (Status == MAX_UINT16) {
      DEBUG ((DEBUG_ERROR, "SPRD-USB: SC2730 Type-C status read failed while waiting for source attach\n"));
      return FALSE;
    }
    if ((Status & SC2730_TYPEC_STATE_MASK) == SC2730_TYPEC_ATTACHED_SRC) {
      break;
    }
  }

  Mode = SprdAdiRead16 (SPRD_SC2730_TYPEC_REG + SC2730_TYPEC_MODE);
  DEBUG ((DEBUG_INFO,
    "SPRD-USB: SC2730 Type-C forced source after %lu ms: MODE=0x%04x STATUS=0x%04x state=%u attached-source=%u\n",
    ((Retry < 50) ? (Retry + 1) : 50) * 10, Mode, Status,
    Status & SC2730_TYPEC_STATE_MASK,
    (Status & SC2730_TYPEC_STATE_MASK) == SC2730_TYPEC_ATTACHED_SRC));
  return (Status & SC2730_TYPEC_STATE_MASK) == SC2730_TYPEC_ATTACHED_SRC;
}

STATIC
EFI_STATUS
SprdFan54015ReadRegister (
  IN  EFI_I2C_MASTER_PROTOCOL  *I2c,
  IN  UINT8                    Register,
  OUT UINT8                    *Value
  )
{
  EFI_STATUS              Status;
  EFI_I2C_REQUEST_PACKET  *Packet;
  UINTN                   PacketSize;

  PacketSize = sizeof (EFI_I2C_REQUEST_PACKET) + sizeof (EFI_I2C_OPERATION);
  Packet = AllocateZeroPool (PacketSize);
  if (Packet == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Packet->OperationCount = 2;
  Packet->Operation[0].Flags         = 0;
  Packet->Operation[0].LengthInBytes = 1;
  Packet->Operation[0].Buffer        = &Register;
  Packet->Operation[1].Flags         = I2C_FLAG_READ;
  Packet->Operation[1].LengthInBytes = 1;
  Packet->Operation[1].Buffer        = Value;
  Status = I2c->StartRequest (I2c, PcdSprdUsbFan54015Address, Packet, NULL, NULL);
  FreePool (Packet);
  return Status;
}

STATIC
EFI_STATUS
SprdFan54015WriteRegister (
  IN EFI_I2C_MASTER_PROTOCOL  *I2c,
  IN UINT8                    Register,
  IN UINT8                    Value
  )
{
  EFI_STATUS              Status;
  EFI_I2C_REQUEST_PACKET  *Packet;
  UINTN                   PacketSize;
  UINT8                   Data[2];

  PacketSize = sizeof (EFI_I2C_REQUEST_PACKET) + sizeof (EFI_I2C_OPERATION);
  Packet = AllocateZeroPool (PacketSize);
  if (Packet == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Data[0] = Register;
  Data[1] = Value;
  Packet->OperationCount = 1;
  Packet->Operation[0].Flags         = 0;
  Packet->Operation[0].LengthInBytes = sizeof (Data);
  Packet->Operation[0].Buffer        = Data;
  Status = I2c->StartRequest (I2c, PcdSprdUsbFan54015Address, Packet, NULL, NULL);
  FreePool (Packet);
  return Status;
}

STATIC
EFI_STATUS
SprdFan54015KeepAlive (
  IN BOOLEAN  Report
  )
{
  EFI_STATUS                Status;
  SPRD_AON_I2C_PROTOCOL     *AonI2c;
  EFI_I2C_MASTER_PROTOCOL   *I2c;
  UINT8                     Control0;
  UINT8                     Control1;
  UINT8                     DesiredControl1;

  Status = gBS->LocateProtocol (&gSprdAonI2cProtocolGuid, NULL, (VOID **)&AonI2c);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  I2c = AonI2c->I2c;
  Status = SprdFan54015ReadRegister (I2c, FAN54015_REG_CONTROL0, &Control0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SprdFan54015WriteRegister (
             I2c,
             FAN54015_REG_CONTROL0,
             (UINT8)(Control0 | FAN54015_RESET_WATCHDOG)
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SprdFan54015ReadRegister (I2c, FAN54015_REG_CONTROL1, &Control1);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /*
    This is the same REG01 value programming as fan54015_charger_enable_otg():
    enable OPA/boost and clear HZ mode.  Reapply it if the charger reset it.
  */
  DesiredControl1 = (UINT8)((Control1 | FAN54015_OTG_ENABLE) & ~FAN54015_HZ_MODE);
  if (DesiredControl1 != Control1) {
    DEBUG ((DEBUG_WARN, "SPRD-USB: restoring FAN54015 OTG/HZ state (REG01=0x%02x)\n", Control1));
    Control1 = DesiredControl1;
    Status = SprdFan54015WriteRegister (I2c, FAN54015_REG_CONTROL1, Control1);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (Report) {
    DEBUG ((DEBUG_INFO,
      "SPRD-USB: FAN54015 keep-alive: REG00=0x%02x REG01=0x%02x (OTG=%u)\n",
      Control0, Control1, (Control1 & FAN54015_OTG_ENABLE) != 0));
  }
  return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
SprdFan54015WatchdogHandler (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS  Status;

  (VOID)Event;
  (VOID)Context;
  Status = SprdFan54015KeepAlive (TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: FAN54015 watchdog keep-alive failed: %r\n", Status));
  }
}

STATIC
EFI_STATUS
EFIAPI
SprdUsbSetVbus (
  IN SPRD_USB_INIT_PROTOCOL  *This,
  IN BOOLEAN                 Enable
  )
{
  EFI_STATUS                Status;
  SPRD_AON_I2C_PROTOCOL     *AonI2c;
  EFI_I2C_MASTER_PROTOCOL   *I2c;
  EFI_I2C_REQUEST_PACKET    *Packet;
  UINT8                     Data[2];
  UINTN                     PacketSize;

  Status = gBS->LocateProtocol (&gSprdAonI2cProtocolGuid, NULL, (VOID **)&AonI2c);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: AON I2C protocol unavailable: %r\n", Status));
    return Status;
  }
  I2c = AonI2c->I2c;
  DEBUG ((DEBUG_VERBOSE, "SPRD-USB: VBUS %a via FAN54015@0x%02x\n",
    Enable ? "enable" : "disable", PcdSprdUsbFan54015Address));

  PacketSize = sizeof (EFI_I2C_REQUEST_PACKET) + sizeof (EFI_I2C_OPERATION);
  Packet = AllocateZeroPool (PacketSize);
  if (Packet == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  /* Read-modify-write FAN54015 REG01; other charger control bits are live. */
  Data[0] = FAN54015_REG_CONTROL1;
  Packet->OperationCount = 2;
  Packet->Operation[0].Flags         = 0;
  Packet->Operation[0].LengthInBytes = 1;
  Packet->Operation[0].Buffer        = &Data[0];
  Packet->Operation[1].Flags         = I2C_FLAG_READ;
  Packet->Operation[1].LengthInBytes = 1;
  Packet->Operation[1].Buffer        = &Data[1];
  Status = I2c->StartRequest (I2c, PcdSprdUsbFan54015Address, Packet, NULL, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: FAN54015 REG01 read failed: %r\n", Status));
    FreePool (Packet);
    return Status;
  }

  /* Match Linux: REG01 OPA/OTG boost on, HZ mode off. */
  Data[0] = FAN54015_REG_CONTROL1;
  if (Enable) {
    Data[1] |= FAN54015_OTG_ENABLE;
    Data[1] &= (UINT8)~FAN54015_HZ_MODE;
  } else {
    Data[1] &= (UINT8)~FAN54015_OTG_ENABLE;
  }
  Packet->OperationCount = 1;
  Packet->Operation[0].Flags         = 0;
  Packet->Operation[0].LengthInBytes = sizeof (Data);
  Packet->Operation[0].Buffer        = Data;
  Status = I2c->StartRequest (I2c, PcdSprdUsbFan54015Address, Packet, NULL, NULL);
  FreePool (Packet);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: FAN54015 REG01 write failed: %r\n", Status));
  }
  return Status;
}

STATIC
VOID
SprdUsbPhySupplyEnable (
  VOID
  )
{
  UINT16  PowerDown;
  UINT16  Voltage;

  /* hsphy in ums9620-1h10.dts is supplied by the SC2730 VDDUSB33 LDO. */
  PowerDown = SprdAdiRead16 (SPRD_SC2730_LDO_USB33_PD_OFFSET);
  Voltage   = SprdAdiRead16 (SPRD_SC2730_LDO_USB33_VOL_OFFSET);
  if ((PowerDown == MAX_UINT16) || (Voltage == MAX_UINT16)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: cannot read SC2730 VDDUSB33 registers\n"));
    return;
  }

  /* 3.3 V must be stable before releasing the USB2 PHY isolation/reset. */
  Voltage = (UINT16)((Voltage & ~SPRD_SC2730_LDO_USB33_VOL_MASK) |
                     SPRD_SC2730_LDO_USB33_3V3);
  MmioWrite32 (SPRD_SC2730_LDO_USB33_VOL_REG, Voltage);
  MmioWrite32 (SPRD_SC2730_LDO_USB33_PD_REG,
    (UINT16)(PowerDown & ~SPRD_SC2730_LDO_USB33_PD));
  MicroSecondDelay (1000);

  PowerDown = SprdAdiRead16 (SPRD_SC2730_LDO_USB33_PD_OFFSET);
  Voltage   = SprdAdiRead16 (SPRD_SC2730_LDO_USB33_VOL_OFFSET);
  DEBUG ((DEBUG_INFO,
    "SPRD-USB: SC2730 VDDUSB33: PD=0x%04x VOL=0x%04x enabled=%u\n",
    PowerDown, Voltage,
    (PowerDown != MAX_UINT16) &&
    ((PowerDown & SPRD_SC2730_LDO_USB33_PD) == 0)));
}

STATIC
VOID
SprdUsbPhyInit (
  VOID
  )
{
  UINTN  Aon;
  UINTN  Phy;

  Aon = SPRD_AON_APB_BASE;
  Phy = SPRD_ANLG_PHY_G2_BASE;
  DEBUG ((DEBUG_INFO, "SPRD-USB: initializing USB2 PHY, AON=0x%lx PHY=0x%lx\n", Aon, Phy));

  SprdUsbPhySupplyEnable ();

  /* Keep U-Boot's core clock alive and enable the UTMI/analog PHY gates. */
  MmioOr32 (Aon + AON_APB_EB1, AON_OTG_UTMI_EB | AON_ANA_EB);
  MmioOr32 (Aon + AON_CGM_REG1, AON_CGM_OTG_REF_EN | AON_CGM_DPHY_REF_EN);

  /* Power up and take the USB2 PHY out of isolation. */
  MmioAnd32 (Phy + USB20_ISO_SW, ~USB20_ISO_SW_EN);
  MmioAnd32 (Phy + USB20_BATTER_PLL, ~(USB20_PS_PD_L | USB20_PS_PD_S));
  MmioOr32 (Aon + AON_OTG_PHY_TEST, AON_OTG_VBUS_VALID_PHYREG);
  MmioOr32 (Phy + USB20_UTMI_CTL1, USB20_VBUSVLDEXT | USB20_DATABUS16_8);
  MmioOr32 (Aon + AON_OTG_PHY_CTRL, AON_UTMI_WIDTH_SEL);
  SprdMmioAndThenOr32 (Phy + USB20_TRIMMING, ~(USB20_TUNEHSAMP | USB20_TFREGRES),
                    USB20_TUNEHSAMP_2_6MA | USB20_TFREGRES_TUNE_VALUE);

  /* Hardware requires a 20-30 ms PHY/UTMI reset pulse. */
  MmioOr32 (Aon + AON_APB_RST1, AON_OTG_PHY_SOFT_RST | AON_OTG_UTMI_SOFT_RST);
  MicroSecondDelay (25000);
  MmioAnd32 (Aon + AON_APB_RST1, ~(AON_OTG_PHY_SOFT_RST | AON_OTG_UTMI_SOFT_RST));
  MicroSecondDelay (1000);

  /* Force A-device semantics; this is the required U-Boot gadget takeover. */
  MmioAnd32 (Aon + AON_OTG_PHY_CTRL, ~AON_USB2_PHY_IDDIG);
  MmioOr32 (Phy + USB20_REG_SEL_CFG_0, USB20_REG_SEL_DPPULLDOWN | USB20_REG_SEL_DMPULLDOWN);
  MmioOr32 (Phy + USB20_UTMI_CTL2, USB20_DPPULLDOWN | USB20_DMPULLDOWN);
  MmioOr32 (Phy + USB20_UTMI_CTL1, 0x200U); // Kernel USB20_RESERVED host value.
  DEBUG ((DEBUG_INFO,
    "SPRD-USB: PHY host mode: PHY_CTRL=0x%08x UTMI_CTL1=0x%08x UTMI_CTL2=0x%08x\n",
    MmioRead32 (Aon + AON_OTG_PHY_CTRL),
    MmioRead32 (Phy + USB20_UTMI_CTL1),
    MmioRead32 (Phy + USB20_UTMI_CTL2)));
}

STATIC
VOID
SprdMusbStartHostSession (
  VOID
  )
{
  UINTN  Base;
  UINTN  Retry;
  UINT8  Value;

  Base = SPRD_MUSB_BASE;
  DEBUG ((DEBUG_INFO, "SPRD-USB: taking MUSB 0x%lx from U-Boot gadget mode\n", Base));
  DEBUG ((DEBUG_INFO, "SPRD-USB: MUSB before takeover: DEVCTL=0x%02x POWER=0x%02x OTG_EXT=0x%02x\n",
    MmioRead8 (Base + MUSB_DEVCTL), MmioRead8 (Base + MUSB_POWER),
    MmioRead8 (Base + MUSB_OTG_EXT_CSR)));
  /*
    U-Boot runs this MUSB instance as a gadget.  In particular, TESTMODE,
    FADDR, and the vendor TX/RX buffer state survive the PHY reset.  Linux
    clears TESTMODE in sprd_musb_disable() before it starts a host session.
    Do the equivalent here before exposing the controller to UsbBusDxe.
  */
  DEBUG ((DEBUG_INFO,
    "SPRD-USB: MUSB stale state: TESTMODE=0x%02x FADDR=0x%02x OTG_EXT=0x%02x\n",
    MmioRead8 (Base + MUSB_TESTMODE), MmioRead8 (Base + MUSB_FADDR),
    MmioRead8 (Base + MUSB_OTG_EXT_CSR)));
  MmioWrite8 (Base + MUSB_TESTMODE, 0);
  MmioWrite8 (Base + MUSB_FADDR, 0);

  /* Disconnect the U-Boot gadget and acknowledge all stale interrupt state. */
  MmioWrite8 (Base + MUSB_POWER, 0);
  MmioWrite8 (Base + MUSB_INTRUSBE, 0);
  MmioWrite16 (Base + MUSB_INTRTXE, 0);
  MmioWrite16 (Base + MUSB_INTRRXE, 0);
  (VOID)MmioRead8 (Base + MUSB_INTRUSB);
  (VOID)MmioRead16 (Base + MUSB_INTRTX);
  (VOID)MmioRead16 (Base + MUSB_INTRRX);

  /*
    CLEAR_TXBUFF/CLEAR_RXBUFF are vendor one-shot requests.  Pulse them with
    host-force still disabled, then restore only the two bits required by the
    host path.  Keeping arbitrary U-Boot OTG_EXT bits is unsafe here.
  */
  MmioWrite8 (Base + MUSB_OTG_EXT_CSR, MUSB_CLEAR_TXBUFF | MUSB_CLEAR_RXBUFF);
  MicroSecondDelay (10);
  MmioWrite8 (Base + MUSB_OTG_EXT_CSR, 0);
  MicroSecondDelay (10);

  Value = MmioRead8 (Base + MUSB_DEVCTL);
  MmioWrite8 (Base + MUSB_DEVCTL, Value | MUSB_DEVCTL_SESSION);
  MmioWrite8 (Base + MUSB_OTG_EXT_CSR, MUSB_HOST_FORCE_EN | MUSB_TX_CMPL_MODE);
  /* Match musb_start(): keep ISO update and high-speed bus detection alive. */
  MmioOr8 (Base + MUSB_POWER, MUSB_POWER_ISOUPDATE | MUSB_POWER_HSENAB);

  /*
    This must precede VBUS boost.  It is the same ordering used by
    sprd_musb_set_vbus(): SESSION causes the core to leave the U-Boot B-device
    state, then the board enables VBUS for the resulting A-device session.
  */
  for (Retry = 0; Retry < 100; Retry++) {
    if ((MmioRead8 (Base + MUSB_DEVCTL) & MUSB_DEVCTL_BDEVICE) == 0) {
      break;
    }
    MicroSecondDelay (1000);
  }
  /* U-Boot gadget mode can reassert this device-only bit during takeover. */
  MmioAnd8 (Base + MUSB_POWER, (UINT8)~MUSB_POWER_SOFTCONN);
  DEBUG ((DEBUG_INFO,
    "SPRD-USB: MUSB host session started after %lu ms: DEVCTL=0x%02x (BDEVICE=%u HM=%u) POWER=0x%02x OTG_EXT=0x%02x TESTMODE=0x%02x FADDR=0x%02x\n",
    Retry,
    MmioRead8 (Base + MUSB_DEVCTL),
    (MmioRead8 (Base + MUSB_DEVCTL) & MUSB_DEVCTL_BDEVICE) != 0,
    (MmioRead8 (Base + MUSB_DEVCTL) & MUSB_DEVCTL_HM) != 0,
    MmioRead8 (Base + MUSB_POWER), MmioRead8 (Base + MUSB_OTG_EXT_CSR),
    MmioRead8 (Base + MUSB_TESTMODE), MmioRead8 (Base + MUSB_FADDR)));
}

EFI_STATUS
EFIAPI
SprdUsbInitDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS              Status;
  SPRD_USB_INIT_PROTOCOL  *Init;
  EFI_HANDLE              Handle;

  Init = AllocateZeroPool (sizeof (*Init));
  if (Init == NULL) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: cannot allocate init protocol\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((DEBUG_INFO, "SPRD-USB: starting USB initialization DXE\n"));
  SprdUsbPhyInit ();
  Init->MusbBase = SPRD_MUSB_BASE;
  Init->MusbSize = SPRD_MUSB_SIZE;
  Init->SetVbus  = SprdUsbSetVbus;
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (&Handle, &gSprdUsbInitProtocolGuid,
                  EFI_NATIVE_INTERFACE, Init);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: init protocol installation failed: %r\n", Status));
    FreePool (Init);
    return Status;
  }

  /*
    Make the board's CC controller advertise source before taking the MUSB
    core out of U-Boot gadget mode.  A USB-C OTG hub will otherwise retain
    its data switch even if FAN54015 is already supplying 5 V on VBUS.
  */
  if (!SprdTypecForceSource ()) {
    DEBUG ((DEBUG_WARN,
      "SPRD-USB: Type-C source is not attached; continuing for a later hot-plug\n"));
  }
  SprdMusbStartHostSession ();

  /* A failure here means platform I2C is not yet dispatched; MUSB retries it. */
  Status = Init->SetVbus (Init, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SPRD-USB: VBUS enable failed: %r\n", Status));
  } else {
    /* Match the initial watchdog service required by the Linux driver. */
    MicroSecondDelay (50000);
    Status = SprdFan54015KeepAlive (TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SPRD-USB: initial FAN54015 keep-alive failed: %r\n", Status));
    }

    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    SprdFan54015WatchdogHandler,
                    NULL,
                    &mFan54015WatchdogEvent
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SPRD-USB: cannot create FAN54015 watchdog timer: %r\n", Status));
    } else {
      Status = gBS->SetTimer (
                      mFan54015WatchdogEvent,
                      TimerPeriodic,
                      FAN54015_WATCHDOG_PERIOD_100NS
                      );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "SPRD-USB: cannot arm FAN54015 watchdog timer: %r\n", Status));
        gBS->CloseEvent (mFan54015WatchdogEvent);
        mFan54015WatchdogEvent = NULL;
      } else {
        DEBUG ((DEBUG_INFO, "SPRD-USB: FAN54015 watchdog timer armed (5 s)\n"));
      }
    }
  }
  MicroSecondDelay (10000);
  return EFI_SUCCESS;
}