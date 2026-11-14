/** @file
  SprdClock.h

  UMS512 clock-controller definitions.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef __SPRD_CLOCK_DXE_H__
#define __SPRD_CLOCK_DXE_H__

#include <Uefi.h>

/* UMS512 / Sharkl5Pro gate controller addresses from the vendor clock tree. */
#define AON_APB_BASE          0x327D0000U
#define AP_AHB_BASE           0x20100000U
#define AP_APB_BASE           0x71000000U
#define AON_APB_EB0           0x0000U
#define AON_APB_EB1           0x0004U
#define AON_APB_EB2           0x0008U
#define AP_AHB_EB             0x0000U
#define AP_APB_EB             0x0000U

typedef enum {
  CLK_AON_EFUSE = 0, CLK_AON_GPIO, CLK_AON_KPD, CLK_AON_AON_SYST,
  CLK_AON_AP_SYST, CLK_AON_AON_TMR, CLK_AON_SPLK, CLK_AON_PIN,
  CLK_AON_ADI, CLK_AON_EIC, CLK_AON_APB_I2C,
  CLK_AP_AHB_DSI, CLK_AP_AHB_DISPC, CLK_AP_AHB_VSP,
  CLK_AP_AHB_DMA_PUB, CLK_AP_AHB_DMA,
  CLK_AP_APB_SIM0, CLK_AP_APB_IIS0, CLK_AP_APB_APBREG, CLK_AP_APB_SPI0,
  CLK_AP_APB_SPI1, CLK_AP_APB_SPI2, CLK_AP_APB_SPI3, CLK_AP_APB_I2C0,
  CLK_AP_APB_I2C1, CLK_AP_APB_I2C2, CLK_AP_APB_I2C3, CLK_AP_APB_I2C4,
  CLK_AP_APB_UART0, CLK_AP_APB_UART1, CLK_AP_APB_UART2,
  CLK_AP_APB_SDIO0, CLK_AP_APB_SDIO1, CLK_AP_APB_SDIO2,
  CLK_AP_APB_EMMC, CLK_AP_APB_SDIO0_32K, CLK_AP_APB_SDIO1_32K,
  CLK_AP_APB_SDIO2_32K, CLK_AP_APB_EMMC_32K,
  CLK_AON_EMMC_2X, CLK_AON_EMMC_1X,
  CLK_AON_OTG_UTMI, CLK_AON_OTG_PHY, CLK_AON_OTG_REF,
  CLK_ID_MAX
} SPRD_CLOCK_ID;

#endif