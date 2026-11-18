#ifndef __APP_CHARGE_H
#define __APP_CHARGE_H
#include "hc32_ll.h"

typedef enum
{
	APP_IDLE,//未连接车辆
	APP_WAIL_AUTH,//车辆已连接，等待刷卡
	APP_WAIT_READY,//	已授权，等待车辆就绪
	APP_CHARGEING,//充电流程状态，当前先模拟
	APP_CMOPETED,//充电完成
	APP_FAULT,//	故障，停止流程
}CHARGE_STATE;
extern CHARGE_STATE char_state;
void App_ChargeTask(void);

#endif