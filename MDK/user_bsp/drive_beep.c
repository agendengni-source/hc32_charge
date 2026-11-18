#include "drive_beep.h"
#include "ser_systick.h"
#define BUZZER_PORT   GPIO_PORT_H
#define BUZZER_PIN    GPIO_PIN_02
#define BeepON GPIO_SetPins(BUZZER_PORT,BUZZER_PIN)
#define BeepOff GPIO_ResetPins(BUZZER_PORT,BUZZER_PIN)
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