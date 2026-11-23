/**
 *******************************************************************************
 ** \brief  CP 信号状态判读服务接口
 **
 ** \file    ser_cp.h
 ** \version V1.0
 **
 ** \note    把 drive_cp.c 采到的 CP 电压(mV)分类为国标状态：
 **          +12V 未连接 / +9V 已连接未就绪 / +6V 车辆就绪(S2闭合)
 ******************************************************************************/
#ifndef __SER_CP_H
#define __SER_CP_H
#include "hc32_ll.h"

/* CP 正电压分类，供业务层组合刷卡与保护信息。 */
typedef enum
{
	CP_UNKNOWN,      // 数据无效、过期或无法识别
CP_DISCONNECTED,// 对应未连接
CP_CONNECTED,    // 对应已连接、未就绪
CP_READY,        // 对应车辆就绪
}CP_STATE;
extern CP_STATE cp_state;
/* 更新 CP 检测结果；只有正电压分类改变时返回 1。 */
uint8_t Ser_CPcheck(void);
extern volatile uint8_t cp_negative_ok;
#endif
