/**
 *******************************************************************************
 ** \brief  串口屏显示服务接口（页面状态与故障码定义）
 **
 ** \file    ser_screenshow.h
 ** \version V1.0
 **
 ** \note    业务层只需写 screenshow.top_state 和 screenshow.showfault，
 **          显示层负责差量刷新；屏端共 4 个页面（main1/charging/over/waring）
 ******************************************************************************/
#ifndef __SER_SCREENSHOW_H
#define __SER_SCREENSHOW_H
#include "hc32_ll.h"
/* 故障页使用的显示原因，供串口屏转换为提示和编号。 */
typedef enum {
	SHOW_NO_Fault = 0,      //无故障
	SHOW_Fault_S2 = 1,      // 0x66 汽车s2开关故障
	SHOW_FAULT_RELAY,       // 0x67 继电器故障
	SHOW_FAULT_EMERGENCY,   // 0x72 急停被按下
	SHOW_FAULT_OVERTEMP,    // 0x74 过温停止充电
	SHOW_FAULT_LEAKAGE,     // 0x75 发生漏电，停止充电
	SHOW_FAULT_LOW_CURRENT, // 0x76 低电流，停止充电
	SHOW_FAULT_OVERCURRENT, // 0x78 过流，停止充电
	SHOW_FAULT_UNDERVOLT,   // 0x7B 欠压停止充电
	SHOW_FAULT_OVERVOLT,    // 0x7D 过压停止充电
	SHOW_FAULT_NETWORK,     // 0x8A 断网停止充电
} ScreenShowFaultSubState;
/* 串口屏可显示的页面，以及用于首次切页判断的无效状态。 */
typedef enum
{
	Normal,//page main1
	Charging,//page charging
	Charged,//page over
	Malfunction,//page waring
	ScreenInvalid      // 只用于记录：还没有显示过页面
}charge_state;
/* 显示层输入：目标页面和需要展示的故障原因。 */
typedef struct
{
	charge_state top_state;
	ScreenShowFaultSubState showfault;

}ScreenState;
extern ScreenState screenshow;
/* 按当前显示状态执行一次串口屏更新。 */
void ScreenState_Show(void);

#endif
