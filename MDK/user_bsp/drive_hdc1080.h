#ifndef __DRIVE_HDC1080_H
#define __DRIVE_HDC1080_H
#include "hc32_ll.h"
typedef struct {
	float fTemp; /* ¡æ */
	float fHumi; /* %RH */
	uint8_t u8Valid;
} HDC1080_Data_t;
void DRV_HDC1080_Config();
void DRV_HDC1080_Poll(HDC1080_Data_t *pstOut);
#endif
