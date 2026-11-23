/**
 *******************************************************************************
 ** \brief  充电回路继电器（L/N 双路）驱动接口
 **
 ** \file    drive_relay.h
 ** \version V1.0
 **
 ** \note    PA4=L继电器控制，PA5=N继电器控制，高电平吸合；
 **          充电时 L/N 先后（同周期）合闸，停止/故障时分闸
 ******************************************************************************/
#ifndef __DRIVE_RELAY_H
#define __DRIVE_RELAY_H
#include "hc32_ll.h"
  #define Relay_L_PORT   GPIO_PORT_A
#define Relay_L_PIN    GPIO_PIN_04

#define Relay_N_PORT   GPIO_PORT_A
#define Relay_N_PIN    GPIO_PIN_05
/* N 继电器吸合/断开（置位/复位 PA5） */
#define NRealyON() GPIO_SetPins(Relay_N_PORT,Relay_N_PIN)
#define NRealyOFF() GPIO_ResetPins(Relay_N_PORT,Relay_N_PIN)

/* L 继电器吸合/断开（置位/复位 PA4） */
#define LRealyON() GPIO_SetPins(Relay_L_PORT,Relay_L_PIN)
#define LRealyOFF() GPIO_ResetPins(Relay_L_PORT,Relay_L_PIN)
void drive_RelayInit(void);             /* PA4/PA5 推挽输出初始化，默认断开 */
en_pin_state_t realaystate_L(void);     /* 读 L 继电器控制脚电平（见函数说明） */
en_pin_state_t realaystate_N(void);     /* 读 N 继电器控制脚电平（见函数说明） */
#endif
