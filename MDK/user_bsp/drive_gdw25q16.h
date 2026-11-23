/**
 *******************************************************************************
 ** \brief  GD25Q16 SPI NOR Flash 驱动接口
 **
 ** \file    drive_gdw25q16.h
 ** \version V1.0
 **
 ** \note    SPI4：PD9=MISO，PD10=MOSI，PD11=SCK，PD8=软件控制的片选CS；
 **          规划用途：充电记录/累计电量掉电保存（尚未实现读写页功能）
 ******************************************************************************/
#ifndef __DRIVE_GD25Q16_H
#define __DRIVE_GD25Q16_H
#include "hc32_ll.h"

#define GD25Q16CS_PORT   GPIO_PORT_D
#define GD25Q16CS_PIN    GPIO_PIN_08
void drive_Gd25q16Init(void);           /* SPI4+CS 初始化（SPI Mode0） */
uint32_t GD25Q_ReadJEDEC_ID(void);      /* 读器件ID(0x9F)：厂家+容量类型 */
#endif
