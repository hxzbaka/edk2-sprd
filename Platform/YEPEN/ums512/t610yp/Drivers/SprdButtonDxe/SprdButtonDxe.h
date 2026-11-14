/** @file
  SprdButtonDxe.h

  Spreadtrum UMS512 button-driver definitions.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#ifndef _SPRD_BUTTON_DXE_H_
#define _SPRD_BUTTON_DXE_H_

/*
 * UMS512 (SharkL5Pro) Buttons Gpio Pin
 *
 * Power   : PMIC EIC pin 1  (active low)  -> 0x140 + 1 = 0x141
 * Vol+    : PMIC EIC pin 4  (active high) -> 0x140 + 4 = 0x144
 * Vol-    : AP GPIO pin 124 (active low)  -> 0x7C
 */
#define GPIO_KEY_VOLUP       0x144
#define GPIO_KEY_VOLDOWN     0x7C
#define GPIO_KEY_POWER       0x141

#endif /* _SPRD_BUTTON_DXE_H_ */