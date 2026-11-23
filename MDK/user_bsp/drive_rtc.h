/**
 *******************************************************************************
 ** \brief  RTC 实时时钟驱动接口
 **
 ** \file    drive_rtc.h
 ** \version V1.0
 **
 ** \note    RTC 时钟源 XTAL32（32.768kHz），24 小时制；
 **          每次上电重新写固定初始时间（2026-11-18 11:59:00）——
 **          TODO：未做掉电保持/校准，正式计费场景需从平台或按键校时
 ******************************************************************************/
#ifndef __DRIVE_RTC_H
#define __DRIVE_RTC_H
#include "hc32_ll.h"


void drive_RtcInit(void);               /* RTC 初始化并写固定初始时间 */
void getrtctime(void);                  /* 读取当前日期时间到全局变量 */
extern stc_rtc_date_t pstcRtcDate;      /* 最近一次读取的日期 */
extern stc_rtc_time_t pstcRtcTime;      /* 最近一次读取的时间 */
#endif
