#include "drive_relay.h"



void drive_RelayInit(void)
{
	stc_gpio_init_t stcGpioInit;
	 /* PA4 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(Relay_L_PORT, Relay_L_PIN, &stcGpioInit);

    /* PA5 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(Relay_N_PORT, Relay_N_PIN, &stcGpioInit);
}
en_pin_state_t realaystate_L(void)
{
	en_pin_state_t flag=0;
	flag=GPIO_ReadInputPins(Relay_L_PORT,Relay_L_PIN);
	
	return flag;
	
}
en_pin_state_t realaystate_N(void)
{
	en_pin_state_t flag=0;
	flag=GPIO_ReadInputPins(Relay_N_PORT,Relay_N_PIN);
	
	return flag;
	
}