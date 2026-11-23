/**
 *******************************************************************************
 ** \brief  BL0939 电能计量芯片驱动接口
 **
 ** \file    drive_bl0939.h
 ** \version V1.0
 **
 ** \note    BL0939 为上海贝岭内置时钟免校准双通道电能计量芯片，本工程硬件配置：
 **          - 电流采样：ZMCT156 电流互感器（变比 2000:1），二次侧接 1.5Ω 负载电阻
 **          - 电压采样：400kΩ / 49.9Ω 电阻分压网络
 **          - 通信接口：SPI3 三线从机（CPOL=0/CPHA=1，无片选，MSB在前）
 **          寄存器地址与通信帧格式详见 BL0939 数据手册 V1.4 §1.5/§3.1
 ******************************************************************************/
#ifndef __DRIVE_BL0939_H
#define __DRIVE_BL0939_H
#include "hc32_ll.h"

/* BL0939 电参量寄存器地址（只读，数据固定按3字节返回，高字节在前） */
#define IA_RMS 0x04     /* A通道电流有效值寄存器，无符号 */
#define V_RMS  0x06     /* 电压有效值寄存器，无符号 */
#define A_WATT 0x08     /* A通道有功功率寄存器，有符号（bit23为符号位，补码） */
#define CFA_CNT 0x0A    /* A通道有功电能脉冲计数寄存器，无符号（正功加/负功减） */
#define ReadId 0x55     /* SPI读操作帧识别字节（0xA5为写操作识别字节） */

/* 电参数全局缓存：由 read_bl0903alldate() 每500ms（NTC_task周期）刷新一次 */
typedef struct
{
	float current_value;        /* A通道电流有效值，单位 A */
	float volate_value;         /* 电压有效值，单位 V */
	float Power_value;          /* A通道有功功率，单位 W */
	float Pulsecharge_value;    /* 上电累计电量（由CFA_CNT换算），单位 kWh */
}Bl0939_value;

uint32_t Meter_GetCnt(void);                /* 读取CFA_CNT原始脉冲计数（统计基准用） */
extern Bl0939_value blo939_value;           /* 电参数全局缓存实例 */
void drive_Bl0939Init(void);                /* SPI3外设与引脚初始化 */
void read_bl0903alldate(void);              /* 刷新全部电参数缓存 */
#endif
