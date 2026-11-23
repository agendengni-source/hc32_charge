/**
 *******************************************************************************
 ** \brief  充电电量/费用统计服务接口（基于 BL0939 脉冲计数）
 **
 ** \file    ser_blo939.h
 ** \version V1.0
 **
 ** \note    统计模型"里程表法"：CFA_CNT 由芯片内部持续积分累加，
 **          开始充电记录基准计数，之后任意时刻取
 **          (当前计数 - 基准) x 每脉冲电量 = 本次电量，
 **          中间漏读几次都不丢电量，结束前再补一次结算保证数据完整
 ******************************************************************************/
#ifndef __SER_BLO939_H
#define __SER_BLO939_H
#include "hc32_ll.h"
/* 一笔充电统计：起始毫秒计数、时长、脉冲相关字段、本次电量和费用。 */
typedef struct {
	uint32_t t_start, seconds, pulses, cnt_last;  /* 起始时刻/时长s/脉冲数/计量基准(CFA_CNT) */
	float    energy, money;                       /* 本次电量kWh / 费用元 */
} ChargeStat;
/* 重置统计并记录本次充电的起点（合闸前调用） */
void Stat_Start(void);
/* 按最新计量数据更新本次充电统计（1s周期调用；离场时再补一次结算） */
void Stat_Update(void) ;
extern ChargeStat g_stat;
#endif
