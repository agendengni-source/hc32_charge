/**
 *******************************************************************************
 ** \brief  调试/串口屏 UART 接口驱动（USART1）
 **
 ** \file    drive_uart.h
 ** \version V1.0
 **
 ** \note    USART1：PD1=RX，PD2=TX，115200-8-N-1，中断收+阻塞发
 **          printf 经 fputc() 重定向到本串口，用于驱动淘晶驰串口屏
 ******************************************************************************/
#ifndef __DRIVE_UART_H
#define __DRIVE_UART_H
#include "hc32_ll.h"


void drive_UartInit(void);      /* USART1 外设初始化（115200, 8N1） */
void drive_IntCfg(void);        /* USART1 中断注册（当前收发均为轮询，未挂业务） */
#endif

