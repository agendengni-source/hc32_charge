#ifndef __SER_SCREENSHOW_H
#define __SER_SCREENSHOW_H
#include "hc32_ll.h"
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
typedef enum
{
	Normal,//page main1
	Charging,//page charging
	Charged,//page over
	Malfunction,//page waring
	ScreenInvalid      // 只用于记录：还没有显示过页面
}charge_state;
typedef struct
{
	charge_state top_state;
	ScreenShowFaultSubState showfault;
	
}ScreenState;
extern ScreenState screenshow;
void ScreenState_Show(ScreenState screenshow);

#endif