/**
 *******************************************************************************
 ** \brief  CP（控制导引）信号驱动接口
 **
 ** \file    drive_cp.h
 ** \version V1.0
 **
 ** \note    依据 GB/T 18487.1：CP 采用 1kHz PWM，占空比表征最大可用电流
 **          （duty%×0.6=电流A，53.3%→32A）；车端通过 S2 开关与电阻网络
 **          改变 CP 电压档位（12V未连接/9V已连接/6V就绪），正负半周分别采样
 ******************************************************************************/
#ifndef __DRIVE_CP_H
#define __DRIVE_CP_H
#include "hc32_ll.h"
#define CP_TMRA_PERIOD  6250    /* TMRA1 周期计数值：6.25MHz/6250=1kHz PWM */
/* CP 电压采样结果（中断内更新，主循环只读） */
typedef struct
{
	volatile uint32_t PB0VALUE;   /* 正电平采样换算值，单位 mV（+12/+9/+6V档） */
	volatile int PB1VALUE;        /* 负电平采样换算值，单位 mV（约-12/-9V，有符号） */
}CP_VOLTAGE;
extern CP_VOLTAGE Cp_voltage;
void drive_cpInit(void);                    /* CP总初始化（中断+ADC+定时器PWM） */
void Cp_SetCompareValue(uint32_t u32Value); /* 动态调整PWM占空比（改电流档位） */
#endif
