#ifndef __DRIVE_RCMU101_H
#define __DRIVE_RCMU101_H
#include "hc32_ll.h"

void drive_Rcmu101Init(void);
uint8_t Self_check(void);
uint8_t REAIL_ERROR(void);
#endif