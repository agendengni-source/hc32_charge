/**
 *******************************************************************************
 ** \brief  射频读卡模块串口驱动接口（USART3，TTL608 模块）
 **
 ** \file    drive_rscard.h
 ** \version V1.0
 **
 ** \note    USART3：PB6=TX，PB7=RX，9600-8-N-1，接收中断逐字节入缓冲，
 **          SysTick 5ms 无新字节即判定一帧结束（帧超时机制）
 ******************************************************************************/
#ifndef __DRIVE_RSCARD_H
#define __DRIVE_RSCARD_H
#include "hc32_ll.h"
#define datelenmax 128      /* 接收缓冲区上限，超出即复位计数防溢出 */
/* 读卡接收状态（中断与 SysTick、解析任务三方访问） */
typedef struct
{
	uint8_t u3rx_datebuf[datelenmax];   /* 接收缓冲区 */
	uint8_t datelen;                    /* 当前帧已收字节数 */
	uint32_t u3overtime;                /* 帧内字节间隔计数（ms，SysTick累加） */
	uint8_t u3flag;                     /* 1=正在收帧（首字节已到） */
	uint8_t u3overtimeflag;             /* 1=帧超时成立，等待解析任务取走 */
//	uint8_t

}u3_state;
extern u3_state u3state;
void drive_u3IntCfg(void);      /* USART3 接收中断注册 */
void drive_RscardInit(void);    /* USART3 外设初始化（9600, 8N1） */
#endif
