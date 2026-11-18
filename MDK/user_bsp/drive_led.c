#include "drive_led.h"
#define led_num 3
LED_objtypedef led_obj[led_num]={
	{LED_RED_PORT,LED_RED_PIN,led_off},
	{LED_GREEN_PORT,LED_GREEN_PIN,led_off},
	{LED_BLUE_PORT,LED_BLUE_PIN,led_off}
};
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

void LedOn(led_id id)
{
	GPIO_SetPins(led_obj[id].LED_Port,led_obj[id].LED_Pin);
}
void LedOff(led_id id)
{
	GPIO_ResetPins(led_obj[id].LED_Port,led_obj[id].LED_Pin);
}
void ledTuen(led_id id)
{

	GPIO_TogglePins(led_obj[id].LED_Port,led_obj[id].LED_Pin);
}