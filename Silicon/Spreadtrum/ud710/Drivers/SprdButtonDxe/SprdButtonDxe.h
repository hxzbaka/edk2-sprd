/** @file
  SprdButtonDxe.h

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef _SPRD_BUTTON_DXE_H_
#define _SPRD_BUTTON_DXE_H_


#define AP_EIC_BASE         0x40210000
#define ADI_BASE            0x40380000
#define PMIC_EIC_BASE       0x280
#define REG_ADI_RD_CMD      0x28
#define REG_ADI_RD_DATA     0x2C
#define BIT_RD_BUSY         (1U << 31)

#endif