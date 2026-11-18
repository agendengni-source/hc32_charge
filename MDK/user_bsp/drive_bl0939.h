#ifndef __DRIVE_BL0939_H
#define __DRIVE_BL0939_H
#include "hc32_ll.h"
#define IA_RMS 0x04
#define V_RMS  0x06
#define A_WATT 0x08
#define CFA_CNT 0x0A
#define ReadId 0x55
typedef struct
{
	float current_value;
	float volate_value;
	float Power_value;
	float Pulsecharge_value;
}Bl0939_value;
extern Bl0939_value blo939_value;
void drive_Bl0939Init(void);
void read_bl0903alldate(void);
#endif