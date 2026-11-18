#ifndef __DRIVE_CP_H
#define __DRIVE_CP_H
#include "hc32_ll.h"
#define CP_TMRA_PERIOD  6250 
typedef struct
{
	volatile uint32_t PB0VALUE;
	volatile int PB1VALUE;
}CP_VOLTAGE;
extern CP_VOLTAGE Cp_voltage;
void drive_cpInit(void);
void Cp_SetCompareValue(uint32_t u32Value);
#endif
