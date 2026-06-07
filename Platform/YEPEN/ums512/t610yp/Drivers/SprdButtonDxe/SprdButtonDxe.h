#ifndef _SPRD_BUTTON_DXE_H_
#define _SPRD_BUTTON_DXE_H_


#define GPIO_KEY_VOLUP_PIN       10
#define GPIO_KEY_VOLDOWN_PIN     2
#define GPIO_KEY_POWER_PIN       1

/*
VOLUP   = 0x140 + 10 = 0x14A
POWER   = 0x140 + 1  = 0x141
VOLDOWN = 0x120 + 2  = 0x122
*/
#define GPIO_KEY_VOLUP      0x14A
#define GPIO_KEY_VOLDOWN    0x122
#define GPIO_KEY_POWER      0x141


#endif