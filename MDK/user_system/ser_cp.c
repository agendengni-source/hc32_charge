/**
 *******************************************************************************
 ** \brief  CP 信号状态判读服务（电压分档 + 负半周二极管检查）
 **
 ** \file    ser_cp.c
 ** \version V1.0
 **
 ** \note    正电压分档（GB/T 18487.1 附录A，允许工程裕量）：
 **          11.4V~12.0V  未连接（无车辆，CP 空载 +12V）
 **          8.0V~10.0V   已连接未就绪（车端 S2 断开，+9V 档）
 **          5.6V~7.0V    车辆就绪（S2 闭合，+6V 档）
 **          负半周检查：-12V~-10V 说明车端反馈二极管正常（cp_negative_ok=1），
 **          作为进入充电的前置条件之一（防二极管缺失故障，故障码7C）
 **          判读节奏：Ser_CPcheck 内部 10ms 最小间隔节流；
 **          只有分类变化才返回 1，业务层据此做状态切换
 ******************************************************************************/
#include "ser_cp.h"
#include "drive_cp.h"
#include "drive_relay.h"
#include "ser_screenshow.h"
#include "s_rscard.h"
#include "ser_led.h"
#include "ser_systick.h"
#include "app_charge.h"


/* 当前和上一次已报告的 CP 正电压分类，用于识别状态变化。 */
CP_STATE cp_state=CP_UNKNOWN;
CP_STATE lastcp_state=CP_UNKNOWN;
static uint32_t s_tick=0;
/* 负半周区间检查结果；当前只在正电压分类变化时刷新。 */
volatile uint8_t cp_negative_ok=0;
/**
 *******************************************************************************
 ** \brief  将 CP 正电压采样值(mV)分类为连接状态
 **
 ** \param  [in] None
 **
 ** \retval CP_STATE  当前分类（UNKNOWN/断开/连接/就绪）
 ** \note   区间带间隙设计：落在 7.0~8.0V、10.0~11.4V 过渡带时判 UNKNOWN，
 **          避免电压在档位边界抖动造成状态来回切换
 ******************************************************************************/
static CP_STATE CP_GetState(void)
{
	uint32_t voltage_mv = Cp_voltage.PB0VALUE;

	if(11400<=voltage_mv&&voltage_mv<=12000)
	{
		return CP_DISCONNECTED;
	}
	else if(8000<=voltage_mv&&voltage_mv<=10000)
	{
		return CP_CONNECTED;
	}
	else if(5600<=voltage_mv&&voltage_mv<=7000)
	{
		return CP_READY;
	}
	else
	{
		return CP_UNKNOWN;
	}

}
/**
 *******************************************************************************
 ** \brief  负半周电压检查（二极管存在性）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PB1 换算值在 -12V~-10V 区间 → 车端二极管正常，cp_negative_ok=1；
 **          否则清 0（二极管缺失时禁止进入充电）
 ******************************************************************************/
static void check(void)
{
	 int32_t voltage_mv = Cp_voltage.PB1VALUE;
	if(-12000<=voltage_mv&&voltage_mv<=-10000)
	{
		cp_negative_ok=1;
	}
	else
	{
		cp_negative_ok=0;
	}
}
/**
 *******************************************************************************
 ** \brief  CP 状态检查入口（由充电状态机每 100ms 调用一次）
 **
 ** \param  [in] None
 **
 ** \retval uint8_t  1:正电压分类发生变化（业务层应处理切换）  0:无变化
 ** \note   内部 10ms 节流：距上次检查不足 10ms 直接返回 0；
 **          分类变化时同步刷新负半周检查结果并更新历史分类
 ******************************************************************************/
uint8_t Ser_CPcheck(void)
{
	uint32_t now=get_systicktime();
	/* 这是最小检查间隔，实际频率取决于调用本函数的任务周期。 */
	if(now-s_tick<10)
	{
		return 0;
	}
	s_tick=now;
	/* 每10ms更新一次检测结果，当前不执行充电控制 */
		/* 保存上一次检测的状态 */
	cp_state=CP_GetState();   /* PB0：判断当前车辆状态 */
	if(lastcp_state!=cp_state)//上一次状态和当前状态不一样，则表明状态变了
	{
	check();
lastcp_state=cp_state;		/* PB1：更新负半周检查结果 */
	return 1;
	}

	return 0;
}
