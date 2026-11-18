#include "drive_dswb04.h"
#define DIP3 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_03)
#define DIP2 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_02)
#define DIP1 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_01)
#define DIP0 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_00)
void drive_Dswb04Init(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
   (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_C, GPIO_PIN_00, &stcGpioInit);

    /* PC1 set to GPIO-Input */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_C, GPIO_PIN_01, &stcGpioInit);
    /* PC2 set to GPIO-Input */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_C, GPIO_PIN_02, &stcGpioInit);
    /* PC3 set to GPIO-Input */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_C, GPIO_PIN_03, &stcGpioInit);

}
uint8_t DRV_dip_Read(void)
{
	if((DIP3==RESET)&&DIP2==RESET)
	{
		if((DIP1==RESET)&&DIP0==RESET)
		{
			
			return 1;
		}
	
	}
	return 0;
}