/**
 *******************************************************************************
 ** \brief  状态指示灯语服务接口（三色灯语义化显示）
 **
 ** \file    ser_led.h
 ** \version V1.0
 **
 ** \note    灯语规则（由 car_state/Charging_issue/carcon_state 驱动）：
 **          未连接：蓝灯慢闪(联网)/常亮(离线)
 **          已连接未充电：绿灯常亮；充电中：绿蓝交替闪；完成：绿灯常亮
 **          故障：红灯常亮(急停) 或按故障类型闪烁 N 次（见 FaultCode_t）
 ******************************************************************************/
#ifndef __SER_LED_H
#define __SER_LED_H
#include "hc32_ll.h"
/* 灯语主状态：未连接、已连接或故障。 */
typedef enum
{
	Car_NCON,
	Car_CON,
	Car_Issue,
}Car_state;
/* 车辆未连接时使用的联网状态。 */
typedef enum
{
	CT_internet,
	NCT_internet,
}INT_State;
/* 已连接车辆使用的灯语子状态。 */
typedef enum
{
	Plugged_ncharged,
	charging,
	compete,
}Carcon_state;
/* 业务故障原因及对应的红灯显示规则。 */
typedef enum
{
    FAULT_EMERGENCY_STOP,               /* 急停故障 常亮 */
    FAULT_UNDER_VOLTAGE,                /* 欠压故障 闪1次*/
    FAULT_OVER_VOLTAGE,                 /* 过压故障 闪2次*/
    FAULT_LEAKAGE,                      /* 漏电故障 闪3次*/
    FAULT_RESERVED_NETWORK,             /* 预留网络异常 闪4次*/
    FAULT_CARD_READER_COMM,             /* 刷卡板通讯异常 闪5次*/
    FAULT_OVER_CURRENT,                 /* 过流故障 闪6次*/
    FAULT_LOW_CURRENT,                  /* 低电流 闪7次*/
    FAULT_OVER_TEMPERATURE,             /* 过温故障 闪8次*/
    FAULT_VEHICLE_S2_SWITCH,            /* 汽车 S2 开关异常 闪9次*/
    FAULT_RELAY_WELDED,                 /* 继电器粘连故障 闪10次*/
    FAULT_GUN_SHORT_CIRCUIT,            /* 枪短路故障 闪12次*/
    FAULT_DIODE_MISSING,                /* 二极管不存在 闪13次*/
    FAULT_GROUND,                       /* 接地故障 闪11次*/
}FaultCode_t;
extern FaultCode_t Charging_issue;
extern Car_state car_state;
extern INT_State int_state;
extern Carcon_state carcon_state;
/* 按当前灯语状态推进一次显示，由 LED 任务周期调用。 */
void led_Language(void);
/* 绿蓝交替闪半拍周期（ms）；实际节奏受 LED_task 任务周期影响 */
#define Alternat_flashtime 300
#endif
