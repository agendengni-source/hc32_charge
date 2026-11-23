/**
 *******************************************************************************
 ** \brief  急停按钮输入驱动
 **
 ** \file    drive_emstop.c
 ** \version V1.0
 **
 ** \note    PC8 配置为数字输入（未启用内部上下拉，电平由外部电路决定）；
 **          按钮消抖状态机在服务层 ser_estop.c 的 Key_state() 中实现
 ******************************************************************************/
#include "drive_emstop.h"



/**
 *******************************************************************************
 ** \brief  急停按钮 GPIO 初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void drive_EmstopInit(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    /* PH2 set to GPIO-Output */
 (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(EMSTOP_PORT, EMSTOP_PIN, &stcGpioInit);
}
