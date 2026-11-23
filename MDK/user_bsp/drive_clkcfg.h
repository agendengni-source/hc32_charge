/**
 *******************************************************************************
 ** \brief  系统时钟配置驱动接口
 **
 ** \file    drive_clkcfg.h
 ** \version V1.0
 **
 ** \note    XTAL(8MHz) → PLL×50/2 → 系统时钟 200MHz（HC32F460 最高主频）
 ******************************************************************************/
#ifndef __DRIVE_CLKCFG_H
#define __DRIVE_CLKCFG_H
#include "hc32_ll.h"


void drive_ClkcfgInit(void);    /* 总线分频+等待周期+XTAL/PLL+时钟源切换 */
#endif
