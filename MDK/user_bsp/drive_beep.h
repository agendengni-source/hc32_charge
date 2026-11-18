#ifndef __DRIVE_BEEP_H
#define __DRIVE_BEEP_H
#include "hc32_ll.h"

void drive_BeepInit(void);
void beepon(uint32_t us);
void beeptask(void);
#endif