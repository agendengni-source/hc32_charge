#ifndef __DRIVE_RSCARD_H
#define __DRIVE_RSCARD_H
#include "hc32_ll.h"
#define datelenmax 128
typedef struct
{
	uint8_t u3rx_datebuf[datelenmax];
	uint8_t datelen;
	uint32_t u3overtime;
	uint8_t u3flag;
	uint8_t u3overtimeflag;
//	uint8_t 
	
}u3_state;
extern u3_state u3state;
void drive_u3IntCfg(void);
void drive_RscardInit(void);
#endif