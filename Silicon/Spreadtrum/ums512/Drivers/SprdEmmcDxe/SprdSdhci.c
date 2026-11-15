/** @file
  SprdSdhci.c

  Copyright (c) 2026, kanadenadi. All rights reserved.<BR>
  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include "SprdEmmcDxe.h"

EFI_STATUS
SprdClockInit (VOID)
{
  UINT32 Val;

  // Enable AHB gate for eMMC module
  Val = MmioRead32 (AP_AHB_GATE_BASE + AP_AHB_EMMC_EB_OFFSET);
  if (!(Val & AP_AHB_EMMC_EB_BIT)) {
    Val |= AP_AHB_EMMC_EB_BIT;
    MmioWrite32 (AP_AHB_GATE_BASE + AP_AHB_EMMC_EB_OFFSET, Val);
    MicroSecondDelay (1000);
  }

  // Enable eMMC 2x and 1x clocks in AON gate
  Val = MmioRead32 (AON_GATE_BASE + AON_GATE_EMMC_CLK_OFFSET);
  Val |= AON_GATE_EMMC_2X_EN | AON_GATE_EMMC_1X_EN;
  MmioWrite32 (AON_GATE_BASE + AON_GATE_EMMC_CLK_OFFSET, Val);
  MicroSecondDelay (1000);

  return EFI_SUCCESS;
}

EFI_STATUS
SprdSdhciReset (UINT8 Mask)
{
  UINT32 Timeout;
  UINT8  Val;

  MmioWrite8 (SDHCI_BASE + SDHCI_SOFTWARE_RESET, Mask);

  for (Timeout = 0; Timeout < 100; Timeout++) {
    if ((MmioRead8 (SDHCI_BASE + SDHCI_SOFTWARE_RESET) & (Mask & ~SDHCI_HW_RESET_CARD)) == 0) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay (1000);
  }

  return EFI_TIMEOUT;
}

VOID
SprdSdhciInitConfig (VOID)
{
  UINT32 Val;

  Val = MmioRead32 (SDHCI_BASE + SDHCI_SPRD_REG_DEBOUNCE);
  Val |= SDHCI_SPRD_BIT_DLL_BAK | SDHCI_SPRD_BIT_DLL_VAL;
  MmioWrite32 (SDHCI_BASE + SDHCI_SPRD_REG_DEBOUNCE, Val);
}

STATIC UINT32
CalcDiv (UINT32 BaseClk, UINT32 Clk)
{
  UINT32 Div;

  if (BaseClk <= Clk * 2) {
    return 0;
  }

  Div = BaseClk / (Clk * 2);

  if ((BaseClk / Div) > (Clk * 2)) {
    Div++;
  }

  if (Div % 2) {
    Div = (Div + 1) / 2;
  } else {
    Div = Div / 2;
  }

  if (Div > SDHCI_SPRD_CLK_MAX_DIV) {
    Div = SDHCI_SPRD_CLK_MAX_DIV;
  }

  return Div;
}

EFI_STATUS
SprdSdhciSetClock (UINT32 Clock)
{
  UINT32 Div;
  UINT16 DivReg;
  UINT16 Clk;
  UINT32 Timeout;
  UINT32 Val;
  UINT32 Mask;
  UINT32 DllDlyOffset;

  MmioWrite16 (SDHCI_BASE + SDHCI_CLOCK_CONTROL, 0);

  if (Clock == 0) {
    return EFI_SUCCESS;
  }

  Div = CalcDiv (EMMC_BASE_RATE, Clock);
  DivReg = (UINT16)(((Div & 0x300) >> 2) | ((Div & 0xFF) << 8));

  Clk = DivReg | SDHCI_CLOCK_INT_EN;
  MmioWrite16 (SDHCI_BASE + SDHCI_CLOCK_CONTROL, Clk);

  for (Timeout = 0; Timeout < 200; Timeout++) {
    Clk = MmioRead16 (SDHCI_BASE + SDHCI_CLOCK_CONTROL);
    if (Clk & SDHCI_CLOCK_INT_STABLE) {
      break;
    }
    MicroSecondDelay (1000);
  }

  if (Timeout >= 200) {
    return EFI_TIMEOUT;
  }

  Clk |= SDHCI_CLOCK_CARD_EN;
  MmioWrite16 (SDHCI_BASE + SDHCI_CLOCK_CONTROL, Clk);

  Val = MmioRead32 (SDHCI_BASE + SDHCI_SPRD_REG_BUSY_POSI);
  Mask = SDHCI_SPRD_BIT_OUTR_CLK_AUTO_EN | SDHCI_SPRD_BIT_INNR_CLK_AUTO_EN;
  if (Clock > 400000) {
    Val |= Mask;
  } else {
    Val &= ~Mask;
  }
  MmioWrite32 (SDHCI_BASE + SDHCI_SPRD_REG_BUSY_POSI, Val);

  DllDlyOffset = MmioRead32 (SDHCI_BASE + SDHCI_SPRD_REG_DLL_DLY_OFFSET);
  if (Clock <= 400000) {
    DllDlyOffset |= SDHCI_SPRD_BIT_CMD_DLY_INV | SDHCI_SPRD_BIT_POSRD_DLY_INV;
  } else {
    DllDlyOffset &= ~(SDHCI_SPRD_BIT_CMD_DLY_INV | SDHCI_SPRD_BIT_POSRD_DLY_INV);
  }
  MmioWrite32 (SDHCI_BASE + SDHCI_SPRD_REG_DLL_DLY_OFFSET, DllDlyOffset);

  return EFI_SUCCESS;
}

EFI_STATUS
WaitInhibitClear (UINT32 Mask, UINT32 TimeoutUs)
{
  UINT32 Elapsed;

  for (Elapsed = 0; Elapsed < TimeoutUs; Elapsed += 100) {
    if ((MmioRead32 (SDHCI_BASE + SDHCI_PRESENT_STATE) & Mask) == 0) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay (100);
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
SprdSdhciSendCmd (UINT16 CmdIdx, UINT32 Arg, UINT16 RespType, UINT32 *Resp)
{
  EFI_STATUS Status;
  UINT16     CmdReg;
  UINT32     IntSts;
  UINT32     j;

  Status = WaitInhibitClear (SDHCI_CMD_INHIBIT, 100000);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, SDHCI_INT_ALL_MASK);
  MmioWrite32 (SDHCI_BASE + SDHCI_ARGUMENT, Arg);

  CmdReg = (CmdIdx << 8) | RespType;
  MmioWrite16 (SDHCI_BASE + SDHCI_COMMAND, CmdReg);

  for (j = 0; j < 50000; j++) {
    IntSts = MmioRead32 (SDHCI_BASE + SDHCI_INT_STATUS);
    if (IntSts & SDHCI_INT_ERROR) {
      MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, IntSts);
      return EFI_DEVICE_ERROR;
    }
    if (IntSts & SDHCI_INT_CMD_COMPLETE) {
      MmioWrite32 (SDHCI_BASE + SDHCI_INT_STATUS, SDHCI_INT_CMD_COMPLETE);
      break;
    }
  }

  if (j >= 50000) {
    return EFI_TIMEOUT;
  }

  if (Resp != NULL) {
    if ((RespType & 0x03) == 0x01) {
      Resp[0] = MmioRead32 (SDHCI_BASE + SDHCI_RESPONSE0);
      Resp[1] = MmioRead32 (SDHCI_BASE + SDHCI_RESPONSE1);
      Resp[2] = MmioRead32 (SDHCI_BASE + SDHCI_RESPONSE2);
      Resp[3] = MmioRead32 (SDHCI_BASE + SDHCI_RESPONSE3);
    } else {
      Resp[0] = MmioRead32 (SDHCI_BASE + SDHCI_RESPONSE0);
    }
  }

  return EFI_SUCCESS;
}