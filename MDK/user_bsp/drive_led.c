/**
 *******************************************************************************
 ** \brief  三色状态指示灯驱动
 **
 ** \file    drive_led.c
 ** \version V1.0
 **
 ** \note    红=PB3、蓝=PB4 与 SWD 调试口（TDO/TRST）复用，
 **          初始化时通过 GPIO_SetDebugPort(DISABLE) 释放为普通 IO——
 **          代价是这两脚上的 SWO/Trace 功能不可用（SWD 烧录不受影响）
 ******************************************************************************/
#include "drive_led.h"
#define led_num 3
/* LED 对象表：下标与 led_id 枚举一一对应 */
LED_objtypedef led_obj[led_num]={
	{LED_RED_PORT,LED_RED_PIN,led_off},
	{LED_GREEN_PORT,LED_GREEN_PIN,led_off},
	{LED_BLUE_PORT,LED_BLUE_PIN,led_off}
};
/**
 *******************************************************************************
 ** \brief  三色灯引脚初始化（推挽输出，默认低=灭）
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void drive_LedInit(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    /* PB4 set to GPIO-Output */
    GPIO_SetDebugPort(GPIO_PIN_TRST, DISABLE);
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(LED_BLUE_PORT, LED_BLUE_PIN, &stcGpioInit);

    /* PB3 set to GPIO-Output */
    GPIO_SetDebugPort(GPIO_PIN_TDO, DISABLE);
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(LED_RED_PORT, LED_RED_PIN, &stcGpioInit);

    /* PD7 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(LED_GREEN_PORT, LED_GREEN_PIN, &stcGpioInit);
}

/**
 *******************************************************************************
 ** \brief  点亮指定灯（置位端口）
 **
 ** \param  [in] id  灯编号（LED_RED/LED_GREEN/LED_BLUE）
 **
 ** \retval None
 ******************************************************************************/
void LedOn(led_id id)
{
	GPIO_SetPins(led_obj[id].LED_Port,led_obj[id].LED_Pin);
}
/**
 *******************************************************************************
 ** \brief  熄灭指定灯（复位端口）
 **
 ** \param  [in] id  灯编号
 **
 ** \retval None
 ******************************************************************************/
void LedOff(led_id id)
{
	GPIO_ResetPins(led_obj[id].LED_Port,led_obj[id].LED_Pin);
}
/**
 *******************************************************************************
 ** \brief  翻转指定灯电平（灯语闪烁的基础动作）
 **
 ** \param  [in] id  灯编号
 **
 ** \retval None
 ******************************************************************************/
void ledTuen(led_id id)
{

	GPIO_TogglePins(led_obj[id].LED_Port,led_obj[id].LED_Pin);
}
