/**
 *******************************************************************************
 ** \brief  系统节拍与任务调度器接口（时间片轮询框架核心）
 **
 ** \file    ser_systick.h
 ** \version V1.0
 **
 ** \note    调度模型：SysTick 每 1ms 中断，倒计时到期只置标志并重装周期，
 **          真正的任务函数在主循环 TaskHandle() 中执行——中断里不做重活
 ******************************************************************************/
#ifndef __SER_SYSTICK_H
#define __SER_SYSTICK_H
#include "hc32_ll.h"
/* 调度器的一项任务：执行标志、剩余倒计时、重装周期（ms）和执行入口 */
typedef struct
{
	uint8_t taskflag;       /* 1=已到期待执行（主循环执行后清零） */
	uint32_t tasktime;      /* 倒计时（ms，SysTick中断里递减） */
	uint32_t tasktimeARR;   /* 重装周期（ms，到期后回装此值） */
	void (*taskfun)(void);  /* 任务函数入口 */
}task;
/* 读取当前系统毫秒计数（上电起累计，用于超时/计时差值） */
uint32_t get_systicktime(void);
extern volatile uint32_t beeptime;          /* 蜂鸣器毫秒时基（drive_beep.c使用） */
extern volatile uint32_t led_slowtime[2];   /* LED慢闪计时：[0]=当前值 [1]=周期ms */
/* 启动 SysTick 毫秒节拍（调用前应先 apptaskinit() 注册回调） */
void s_SystickInit(void);
/* 忙等式毫秒延时（基于毫秒计数差值，期间任务全部停摆，仅限初始化阶段使用） */
void SysTick_Delayms(uint32_t time);
/* 注册任务调度回调，应先于 s_SystickInit() 调用 */
void apptaskinit(void);
/* 执行一轮已到期任务，在主循环中反复调用 */
void TaskHandle(void);
#endif
