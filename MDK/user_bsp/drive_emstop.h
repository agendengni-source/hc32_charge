/**
 *******************************************************************************
 ** \brief  急停按钮输入驱动接口
 **
 ** \file    drive_emstop.h
 ** \version V1.0
 **
 ** \note    PC8 输入，按下/释放电平由外部急停开关电路决定（常闭/常开看接线）；
 **          消抖逻辑在服务层 ser_estop.c 中实现（20ms 确认阈值）
 ******************************************************************************/
#ifndef __DRIVE_EMSTOP_H
#define __DRIVE_EMSTOP_H
#include "hc32_ll.h"
#define KEYPRESS GPIO_ReadInputPins(EMSTOP_PORT, EMSTOP_PIN)   /* 读急停按钮电平 */
#define EMSTOP_PORT   GPIO_PORT_C
#define EMSTOP_PIN    GPIO_PIN_08
void drive_EmstopInit(void);
#endif

