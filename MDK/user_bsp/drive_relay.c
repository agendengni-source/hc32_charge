/**
 *******************************************************************************
 ** \brief  充电回路继电器（L/N 双路）驱动
 **
 ** \file    drive_relay.c
 ** \version V1.0
 **
 ** \note    PA4=L继电器，PA5=N继电器，推挽输出、高驱动能力、高电平吸合
 ** \note    TODO：realaystate_x() 读回的是 MCU 输出寄存器电平，
 **          不是继电器真实触点状态；如需"继电器粘连"检测
 **          （FAULT_RELAY_WELDED，故障码67），硬件需增加触点反馈输入
 ******************************************************************************/
#include "drive_relay.h"



/**
 *******************************************************************************
 ** \brief  继电器控制脚初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PA4/PA5 配置为推挽输出（高驱动能力），复位后默认输出低=断开
 ******************************************************************************/
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
/**
 *******************************************************************************
 ** \brief  读 L 继电器控制脚电平
 **
 ** \param  [in] None
 **
 ** \retval en_pin_state_t  SET=输出高（吸合命令有效）  RESET=输出低
 ** \note   回读的是输出电平而非触点状态，不能用于粘连检测
 ******************************************************************************/
en_pin_state_t realaystate_L(void)
{
	en_pin_state_t flag=0;
	flag=GPIO_ReadInputPins(Relay_L_PORT,Relay_L_PIN);

	return flag;

}
/**
 *******************************************************************************
 ** \brief  读 N 继电器控制脚电平
 **
 ** \param  [in] None
 **
 ** \retval en_pin_state_t  SET=输出高（吸合命令有效）  RESET=输出低
 ******************************************************************************/
en_pin_state_t realaystate_N(void)
{
	en_pin_state_t flag=0;
	flag=GPIO_ReadInputPins(Relay_N_PORT,Relay_N_PIN);

	return flag;

}
