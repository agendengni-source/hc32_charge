/**
 *******************************************************************************
 ** \brief  RCMU101 剩余电流传感器（漏电检测模块）驱动接口
 **
 ** \file    drive_rcmu101.h
 ** \version V1.0
 **
 ** \note    U31 = RCMU101SM3-6EI-U（磁通门式剩余电流传感器，5V 供电），
 **          交流 30mA / 直流 6mA 动作，VO 跳闸输出高电平有效。
 **          不使用自检时 CHK 须接地；本电路 CHK 接 MCU，故固件必须实现自检
 ******************************************************************************/
#ifndef __DRIVE_RCMU101_H
#define __DRIVE_RCMU101_H
#include "hc32_ll.h"

void drive_Rcmu101Init(void);   /* PE7(VO输入,上拉)/PE8(CHK输出) GPIO初始化 */
uint8_t Self_check(void);       /* 触发自检：0=通过 1=失败（每笔充电合闸前调用） */
uint8_t REAIL_ERROR(void);      /* 读漏电动作状态：1=VO有效(非自检期间) 0=正常 */
#endif
