#ifndef __DRIVE_EMSTOP_H
#define __DRIVE_EMSTOP_H
#include "hc32_ll.h"

#define EMSTOP_PORT   GPIO_PORT_C
#define EMSTOP_PIN    GPIO_PIN_08
void drive_EmstopInit(void);

#endif