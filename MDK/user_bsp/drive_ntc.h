/**
 *******************************************************************************
 ** \brief  NTC 热敏电阻测温驱动接口（漏电保护器端子温度监测）
 **
 ** \file    drive_ntc.h
 ** \version V1.0
 **
 ** \note    硬件电路：3V3 — R101(5.1kΩ) — PA3(ADC采样点) — NTC — GND
 **          NTC 型号 MF52A104F3950（100kΩ@25℃，B=3950）
 ******************************************************************************/
#ifndef __DRIVE_NTC_H
#define __DRIVE_NTC_H
#include "hc32_ll.h"

void drive_NtcInit(void);       /* PA3/ADC1通道3初始化 */
float NTC_NCT_RValue(void);     /* 读取漏保端子温度，单位℃（B值法，100次均值） */
#endif
