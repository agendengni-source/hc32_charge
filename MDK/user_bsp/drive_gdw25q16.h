#ifndef __DRIVE_GD25Q16_H
#define __DRIVE_GD25Q16_H
#include "hc32_ll.h"

#define GD25Q16CS_PORT   GPIO_PORT_D
#define GD25Q16CS_PIN    GPIO_PIN_08
void drive_Gd25q16Init(void);
uint32_t GD25Q_ReadJEDEC_ID(void);
#endif