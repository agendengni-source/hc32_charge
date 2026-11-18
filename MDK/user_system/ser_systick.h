#ifndef __SER_SYSTICK_H
#define __SER_SYSTICK_H
#include "hc32_ll.h"

uint32_t get_systicktime(void);
extern volatile uint32_t beeptime;
extern volatile uint32_t led_slowtime[2];
void s_SystickInit(void);
void SysTick_Delayms(uint32_t time);
#endif