/**
 *******************************************************************************
 ** \brief  拨码开关（DSWB04 四位）驱动
 **
 ** \file    drive_dswb04.c
 ** \version V1.0
 **
 ** \note    PC0(DIP0)~PC3(DIP3) 四位拨码，低电平=ON；
 **          DRV_dip_Read() 当前只判断"全 0"组合，返回 1/0，
 **          后续可扩展为 4bit 编号读取（0~15）
 ******************************************************************************/
#include "drive_dswb04.h"
#define DIP3 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_03)
#define DIP2 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_02)
#define DIP1 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_01)
#define DIP0 GPIO_ReadInputPins(GPIO_PORT_C,GPIO_PIN_00)
/**
 *******************************************************************************
 ** \brief  拨码开关 GPIO 初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PC0~PC3 数字输入（电平由板上拨码+上下拉决定）
 ******************************************************************************/
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
/**
 *******************************************************************************
 ** \brief  读取拨码组合（当前仅判全 0）
 **
 ** \param  [in] None
 **
 ** \retval uint8_t  1:四位全部为 0（全 ON）  0:其它组合
 ******************************************************************************/
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
