/**
 *******************************************************************************
 ** \brief  充电电量/费用统计服务（"里程表法"计量）
 **
 ** \file    ser_blo939.c
 ** \version V1.0
 **
 ** \note    电量统计模型（对应 BL0939 的 CFA_CNT 脉冲计数寄存器）：
 **          芯片内部持续对功率做时间积分，每积满一份电量输出一个脉冲，
 **          CFA_CNT 就是脉冲计数器（类似老电表的转盘圈数），始终自增；
 **          本模块不做逐秒累加，只在开始充电时记录基准计数，
 **          之后每秒用 (当前计数 - 基准) x 每脉冲电量 刷新统计，
 **          结束/故障离场时由 App_ChargeTask 再补一次结算，保证完整。
 **          相比先换算 float 再相减的方案：无小数截断误差、
 **          读取失败不引入垃圾值、对 24bit 计数回绕天然安全
 ******************************************************************************/
#include "ser_blo939.h"
#include "drive_bl0939.h"
#include "ser_systick.h"
/* 本次充电统计，由业务层更新，由充电中和完成页面读取。 */
ChargeStat g_stat;
/* 电价（元/kWh）：计费策略调整时只改这里（app_charge.c 中另有同名宏，注意同步） */
#define PRICE_PER_KWH  1.20f
/* 每个CFA脉冲对应电量(kWh)：CT 2000:1/1.5Ω、分压400K/49.9Ω，实测校准值。 */
#define KWH_PER_PULSE  4.566e-4f
/**
 *******************************************************************************
 ** \brief  开始一笔充电统计（APP_WAIT_READY 进入 APP_CHARGEING 前调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   记录起始时刻和 CFA_CNT 原始计数作基准（Meter_GetCnt 现场读芯片，
 **          不用 500ms 前的缓存值），并清零本次电量/费用/时长
 ******************************************************************************/
void Stat_Start(void)
{
	g_stat.t_start  = get_systicktime();
	g_stat.pulses   = 0;
	g_stat.cnt_last =  Meter_GetCnt();
	g_stat.energy   = 0.0f;
	g_stat.money    = 0.0f;
	g_stat.seconds  = 0;
}
/**
 *******************************************************************************
 ** \brief  刷新本次充电统计（App_ChargeTask 每 1s 调一次；离场时再补一次）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   电量 = (CFA_CNT - cnt_last) x KWH_PER_PULSE：
 **          差值用 int32_t 有符号数计算，24bit 计数回绕时结果仍正确；
 **          差值为负（芯片异常复位计数清零）时按 0 处理，不倒扣费用
 ******************************************************************************/
void Stat_Update(void)                /* App_ChargeTask 里 1s 调一次 */
{
	uint32_t cnt_now = Meter_GetCnt();
	/* 24位计数用有符号差值，回绕安全；芯片异常复位时差值为负，不倒扣。 */
	int32_t diff = (int32_t)(cnt_now - g_stat.cnt_last);
	if (diff < 0)
	{
		diff = 0;
	}
	g_stat.pulses  = (uint32_t)diff;
	g_stat.energy  = diff * KWH_PER_PULSE;
	g_stat.money   = g_stat.energy*PRICE_PER_KWH;
	g_stat.seconds = (get_systicktime() - g_stat.t_start) / 1000U;
}
