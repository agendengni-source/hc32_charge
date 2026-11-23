/**
 *******************************************************************************
 ** \brief  充电业务状态机接口（应用层核心）
 **
 ** \file    app_charge.h
 ** \version V1.0
 **
 ** \note    状态流转：IDLE -插枪-> WAIL_AUTH -白名单卡-> WAIT_READY
 **          -就绪+自检-> CHARGEING -拔枪/完成-> CMOPETED -5s-> IDLE
 **          任意状态下触发急停/漏电/过温 -> FAULT
 ******************************************************************************/
#ifndef __APP_CHARGE_H
#define __APP_CHARGE_H
#include "hc32_ll.h"

/* 充电业务状态，与 CP 分类和页面状态分别保存。当前充电分支有实际继电器输出。 */
typedef enum
{
	APP_IDLE,//未连接车辆
	APP_WAIL_AUTH,//车辆已连接，等待刷卡
	APP_WAIT_READY,//	已授权，等待车辆就绪
	APP_CHARGEING,//充电流程状态，当前先模拟
	APP_CMOPETED,//充电完成
	APP_FAULT,//	故障，停止流程
}CHARGE_STATE;

/* 急停业务处理入口，由急停任务调用。 */
void stoptask(void);
extern CHARGE_STATE char_state;
/* 周期检查 CP；正电压分类有变化时返回 1。 */
uint8_t Ser_CPcheck(void);
/* 执行一次充电业务状态机并更新灯屏和继电器控制。 */
void App_ChargeTask(void);
#endif
