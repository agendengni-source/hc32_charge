/**
 *******************************************************************************
 ** \brief  拨码开关（DSWB04 四位）驱动接口
 **
 ** \file    drive_dswb04.h
 ** \version V1.0
 **
 ** \note    PC0~PC3 四位拨码输入，低电平=拨到 ON（读到 0）；
 **          用于出厂配置/桩号等硬编码设置
 ******************************************************************************/
#ifndef __DRIVE_DSWB04_H
#define __DRIVE_DSWB04_H
#include "hc32_ll.h"

void drive_Dswb04Init(void);    /* PC0~PC3 数字输入初始化 */
uint8_t DRV_dip_Read(void);     /* 读拨码组合：4位全为0返回1，其余返回0 */
#endif
