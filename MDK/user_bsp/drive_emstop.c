#include "drive_emstop.h"



void drive_EmstopInit(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    /* PH2 set to GPIO-Output */
 (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(EMSTOP_PORT, EMSTOP_PIN, &stcGpioInit);
}