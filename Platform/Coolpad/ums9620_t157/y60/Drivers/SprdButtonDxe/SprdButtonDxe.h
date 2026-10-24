#ifndef _SPRD_BUTTON_DXE_H_
#define _SPRD_BUTTON_DXE_H_

/*
 * UMS9620/Y60 button mapping confirmed against the running kernel:
 *   Power : sc27xx-eic 1, active low
 *   Vol+  : sprd-gpio 7, active low
 *   Vol-  : sc27xx-eic 4, active high
 */
#define GPIO_KEY_VOLUP       0x007
#define GPIO_KEY_VOLDOWN     0x144
#define GPIO_KEY_POWER       0x141

#endif /* _SPRD_BUTTON_DXE_H_ */