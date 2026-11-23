/**
 *******************************************************************************
 ** \brief  三色状态指示灯驱动接口
 **
 ** \file    drive_led.h
 ** \version V1.0
 **
 ** \note    红=PB3（与SWD TDO复用，初始化时关闭调试口功能），
 **          蓝=PB4（与SWD TRST复用），绿=PD7；高电平点亮。
 **          灯语逻辑（闪烁次数表示故障类型）在服务层 ser_led.c 中实现
 ******************************************************************************/
#ifndef __DRIVE_LED_H
#define __DRIVE_LED_H
#include "hc32_ll.h"
/* LED 编号，作为各操作函数的索引 */
typedef enum
{
	LED_RED=0,
	LED_GREEN,
	LED_BLUE,
}led_id;
typedef enum
{
	led_off =0,
	led_on =1,
}LED_StateTypedef;
/* LED 对象：端口+引脚+当前状态（state 仅记录，不参与控制） */
typedef struct
{
	uint8_t LED_Port;
	uint16_t LED_Pin;
	LED_StateTypedef state;
}LED_objtypedef;


#define LED_BLUE_PORT   GPIO_PORT_B
#define LED_BLUE_PIN    GPIO_PIN_04

#define LED_RED_PORT   GPIO_PORT_B
#define LED_RED_PIN    GPIO_PIN_03

#define LED_GREEN_PORT   GPIO_PORT_D
#define LED_GREEN_PIN    GPIO_PIN_07

void drive_LedInit(void);       /* 三色灯引脚初始化（含关闭SWD复用脚） */
void LedOn(led_id id);          /* 点亮指定灯 */
void LedOff(led_id id);         /* 熄灭指定灯 */
void ledTuen(led_id id);        /* 翻转指定灯（闪烁用，函数名沿用Toggle拼写） */
#endif
