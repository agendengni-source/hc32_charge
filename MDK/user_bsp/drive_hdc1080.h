/**
 *******************************************************************************
 ** \brief  HDC1080 温湿度传感器驱动接口（I2C3）
 **
 ** \file    drive_hdc1080.h
 ** \version V1.0
 **
 ** \note    原理图网络 S1（PE2=SDA，PE3=SCL，4.7kΩ 上拉，I2C3 硬件外设）
 **          7bit 器件地址 0x40，14bit 分辨率，温+湿同时转换约需 13ms
 ******************************************************************************/
#ifndef __DRIVE_HDC1080_H
#define __DRIVE_HDC1080_H
#include "hc32_ll.h"

/* 一次轮询得到的温湿度结果 */
typedef struct {
	float fTemp;    /* 温度，单位 ℃（量程 -40~+125℃） */
	float fHumi;    /* 相对湿度，单位 %RH */
	uint8_t u8Valid; /* 1=已读到过有效数据 0=尚未读回 */
	uint8_t u8Err;  /* 连续I2C读失败次数（业务层>5判传感器故障） */
} HDC1080_Data_t;

void DRV_HDC1080_Config();                          /* I2C3 初始化+器件ID校验+分辨率配置 */
void DRV_HDC1080_Poll(HDC1080_Data_t *pstOut);      /* 两阶段轮询读取，由HDC1080_task调用 */
#endif
