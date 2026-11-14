/** @file
  SprdButtonDxe.h

  Definitions for the Spreadtrum button driver.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>
  Copyright (c) 2026, atlas4381. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef _SPRD_BUTTON_DXE_H_
#define _SPRD_BUTTON_DXE_H_


#define AP_EIC_BASE         0x32020000
#define ADI_BASE            0x32100000
#define PMIC_EIC_BASE       0x280
#define REG_ADI_RD_CMD      0x28
#define REG_ADI_RD_DATA     0x2C
#define REG_ADI_WR_CMD      0x24
#define BIT_RD_BUSY         (1U << 31)
#define AP_EIC_MASK_OFFSET  0x04
#define PMIC_EIC_MASK_OFFSET 0x284

#endif