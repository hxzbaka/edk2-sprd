/** @file
  SprdButtonDxe.h

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef _SPRD_BUTTON_DXE_H_
#define _SPRD_BUTTON_DXE_H_

/*
 * UD710 (icc8pro) Buttons Gpio Pin
 *
 * Power   : PMIC EIC pin 1  (active low)  -> 0x140 + 1 = 0x141
 * Vol+    : PMIC EIC pin 4  (active high) -> 0x140 + 4 = 0x144
 * Vol-    : AP GPIO pin 124 (active low)  -> 0x7C
 * AI    : AP GPIO pin 91 (active low)  -> 0x5B
* HAL   : AP GPIO pin 88 (active low)  -> 0x58
 */
#define GPIO_KEY_VOLUP       0x144
#define GPIO_KEY_VOLDOWN     0x7C
#define GPIO_KEY_POWER       0x141

#define GPIO_KEY_AI          0x5B
#define GPIO_KEY_HALL      0x58

#endif /* _SPRD_BUTTON_DXE_H_ */