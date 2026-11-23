/**
 *******************************************************************************
 ** \brief  蜂鸣器驱动接口
 **
 ** \file    drive_beep.h
 ** \version V1.0
 **
 ** \note    PH2 推挽输出，高电平鸣叫；刷卡成功时蜂鸣 100ms 提示。
 **          蜂鸣定时由 beeptime 毫秒计数（SysTick 维护）+ beeptask() 实现
 ******************************************************************************/
#ifndef __DRIVE_BEEP_H
#define __DRIVE_BEEP_H
#include "hc32_ll.h"

void drive_BeepInit(void);      /* PH2 推挽输出初始化，默认低=静音 */
void beepon(uint32_t us);       /* 启动一次蜂鸣（参数实际单位为ms，命名沿用了us） */
void beeptask(void);            /* 到时关蜂鸣，由ESTOP_task每1ms调用 */
#endif
