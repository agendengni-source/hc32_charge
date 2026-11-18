#ifndef __SER_LED_H
#define __SER_LED_H
#include "hc32_ll.h"
typedef enum
{
	Car_NCON,
	Car_CON,
	Car_Issue,
}Car_state;
typedef enum
{
	CT_internet,
	NCT_internet,
}INT_State;
typedef enum
{
	Plugged_ncharged,
	charging,
	compete,
}Carcon_state;
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
void led_Language(void);
#define Alternat_flashtime 300
#endif