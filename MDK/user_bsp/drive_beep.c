/**
 *******************************************************************************
 ** \brief  蜂鸣器驱动（非阻塞定时鸣叫）
 **
 ** \file    drive_beep.c
 ** \version V1.0
 **
 ** \note    时基：beeptime 由 SysTick 每 1ms 加一（ser_systick.c 维护），
 **          beeptask() 由 ESTOP_task 以 1ms 周期调用，到期自动关断；
 **          蜂鸣进行中再次调用 beepon() 不会重启计时（不可重触发）
 ******************************************************************************/
#include "drive_beep.h"
#include "ser_systick.h"
#define BUZZER_PORT   GPIO_PORT_H
#define BUZZER_PIN    GPIO_PIN_02
#define BeepON GPIO_SetPins(BUZZER_PORT,BUZZER_PIN)
#define BeepOff GPIO_ResetPins(BUZZER_PORT,BUZZER_PIN)
/**
 *******************************************************************************
 ** \brief  蜂鸣器 GPIO 初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PH2 推挽输出、高驱动能力；上电默认低电平（静音）
 ******************************************************************************/
void drive_BeepInit(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    /* PH2 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(BUZZER_PORT, BUZZER_PIN, &stcGpioInit);
}
static uint8_t beep_active=0;
static uint32_t beep_start=0;
static uint32_t beep_duration=0;
/**
 *******************************************************************************
 ** \brief  启动一次定时蜂鸣
 **
 ** \param  [in] ms  鸣叫时长（毫秒；形参名 us 为历史命名，实际按 ms 使用）
 **
 ** \retval None
 ** \note   鸣叫中重复调用会被忽略（不重置时长）；记录起点与时长后置位使能
 ******************************************************************************/
void beepon(uint32_t ms)
{
	if(beep_active)
	{
		return;
	}
	beep_start=beeptime;
	beep_duration=ms;
	beep_active=1;
	BeepON;
}
/**
 *******************************************************************************
 ** \brief  蜂鸣到期处理（1ms 任务周期调用，到时关断并清使能）
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void beeptask(void)
{
	if(beep_active)
	{
		if(beeptime-beep_start>=beep_duration)
		{
			BeepOff;
			beep_active=0;
		}

	}
}
