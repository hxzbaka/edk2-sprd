/** @file
  SprdClock.h

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef __SPRD_CLOCK_DXE_H__
#define __SPRD_CLOCK_DXE_H__

#include <Uefi.h>

//
// 时钟域基地址
//
#define AON_CLK_BASE     0x402E0000
#define AP_AHB_CLK_BASE  0x20E00000
#define AP_APB_CLK_BASE  0x71300000

//
// 各个门控寄存器偏移（相对基地址）
//
#define AON_APB_EB0      0x0000
#define AON_APB_EB1      0x0004
#define AON_APB_EB2      0x00B0
#define AP_AHB_EB        0x0000
#define AP_APB_EB        0x0000

//
// 时钟 ID 枚举（涵盖所有已知门控位）
//
typedef enum {
  // AON 域 (EB0)
  CLK_AON_EIC = 0,
  CLK_AON_ADI,
  CLK_AON_EFUSE,
  CLK_AON_GPIO,
  CLK_AON_TPC,
  CLK_AON_FM,
  CLK_AON_ADC,
  CLK_AON_KPD,
  CLK_AON_PWM0,
  CLK_AON_PWM1,
  CLK_AON_PWM2,
  CLK_AON_PWM3,
  CLK_AON_AP_TMR0,
  CLK_AON_AON_TMR,
  CLK_AON_AP_SYST,
  CLK_AON_AON_SYST,
  CLK_AON_AON_INTC,
  CLK_AON_AUD,
  CLK_AON_AUDIF,
  CLK_AON_VBC,
  CLK_AON_PIN,
  CLK_AON_IPI,
  CLK_AON_SPLK,
  CLK_AON_AP_WDG,
  CLK_AON_MM,
  CLK_AON_CKG,
  CLK_AON_GPU,
  CLK_AON_CA53_TS0,
  CLK_AON_WTLCP_INTC,
  CLK_AON_PUBCP_INTC,
  CLK_AON_CA53_DAP,

  // AP AHB 域
  CLK_AP_AHB_DSI,
  CLK_AP_AHB_DISPC,
  CLK_AP_AHB_VSP,
  CLK_AP_AHB_GSP,
  CLK_AP_AHB_OTG,
  CLK_AP_AHB_DMA_PUB,
  CLK_AP_AHB_CE_PUB,
  CLK_AP_AHB_CKG,
  CLK_AP_AHB_SDIO0,
  CLK_AP_AHB_SDIO1,
  CLK_AP_AHB_NANDC,
  CLK_AP_AHB_EMMC,
  CLK_AP_AHB_SPINLOCK,
  CLK_AP_AHB_AHBREG,
  CLK_AP_AHB_EMMC_32K,
  CLK_AP_AHB_SDIO0_32K,
  CLK_AP_AHB_SDIO1_32K,

  // AP APB 域
  CLK_AP_APB_SIM0,
  CLK_AP_APB_IIS0,
  CLK_AP_APB_APBREG,
  CLK_AP_APB_SPI0,
  CLK_AP_APB_SPI2,
  CLK_AP_APB_I2C0,
  CLK_AP_APB_I2C1,
  CLK_AP_APB_I2C2,
  CLK_AP_APB_I2C3,
  CLK_AP_APB_I2C4,
  CLK_AP_APB_UART1,
  CLK_AP_APB_SIM0_32K,
  CLK_AP_APB_INTC0,
  CLK_AP_APB_INTC1,
  CLK_AP_APB_INTC2,
  CLK_AP_APB_INTC3,

  CLK_ID_MAX
} SPRD_CLOCK_ID;

#endif /* __SPRD_CLOCK_DXE_H__ */