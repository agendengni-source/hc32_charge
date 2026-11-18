#include "drive_rcmu101.h"
#include "ser_systick.h"
#define RCMU_VOUT_PORT GPIO_PORT_E
#define RCMU_VOUT_PIN GPIO_PIN_07
#define RCMU_CHECK_PORT GPIO_PORT_E
#define RCMU_CHECK_PIN GPIO_PIN_08

void drive_Rcmu101Init(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_E, GPIO_PIN_07, &stcGpioInit);

    /* PE8 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_E, GPIO_PIN_08, &stcGpioInit);
}
uint8_t Self_check(void)
{
	GPIO_SetPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN);
	SysTick_Delayms(3);
	if(GPIO_ReadInputPins(RCMU_VOUT_PORT,RCMU_VOUT_PIN))
	{
		
		GPIO_ResetPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN);
		SysTick_Delayms(3);
		if(GPIO_ReadInputPins(RCMU_VOUT_PORT,RCMU_VOUT_PIN)==RESET)
		{
			return 1;
			
		}
		return 0;
	}
	GPIO_ResetPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN);
	return 0;
}
uint8_t REAIL_ERROR(void)
{
	uint8_t value=0;
	value=GPIO_ReadInputPins(RCMU_VOUT_PORT,RCMU_VOUT_PIN);
	if(value==1)
	{
		if(GPIO_ReadInputPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}
