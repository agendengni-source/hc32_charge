/**
 *******************************************************************************
 ** \brief  RTC 实时时钟驱动
 **
 ** \file    drive_rtc.c
 ** \version V1.0
 **
 ** \note    时钟源 XTAL32（32.768kHz 外部晶振），24 小时制，无时钟补偿；
 **          注意：每次上电都会重写初始时间（非掉电保持的"首次写入"策略），
 **          getrtctime() 当前未被业务调用（RTC_task 中处于注释状态）
 ******************************************************************************/
#include "drive_rtc.h"



/**
 *******************************************************************************
 ** \brief  RTC 初始化并写固定初始时间
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   流程：RTC_DeInit → 配置时钟源/时制 → 写日期时间 → 启动计数；
 **         DeInit 超时（LL_ERR_TIMEOUT）则跳过整段配置
 ******************************************************************************/
void drive_RtcInit(void)
{
  stc_rtc_init_t stcRtcInit;
    stc_rtc_date_t stcRtcDate;
    stc_rtc_time_t stcRtcTime;
    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT != RTC_DeInit()) {
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc = RTC_CLK_SRC_XTAL32;
        stcRtcInit.u8HourFormat = RTC_HOUR_FMT_24H;
        stcRtcInit.u8ClockCompen = RTC_CLK_COMPEN_DISABLE;
        (void)RTC_Init(&stcRtcInit);

        /* Update date and time */
        /* Date configuration */
        stcRtcDate.u8Year = 26U;
        stcRtcDate.u8Month = RTC_MONTH_NOVEMBER;
        stcRtcDate.u8Day = 18U;
        stcRtcDate.u8Weekday = RTC_WEEKDAY_WEDNESDAY;

        /* Time configuration */
        stcRtcTime.u8Hour = 11U;
        stcRtcTime.u8Minute = 59U;
        stcRtcTime.u8Second = 0U;
        stcRtcTime.u8AmPm = RTC_HOUR_24H;

        if (LL_OK != RTC_SetDate(RTC_DATA_FMT_DEC, &stcRtcDate)) {
            return;
        }

        if (LL_OK != RTC_SetTime(RTC_DATA_FMT_DEC, &stcRtcTime)) {
            return;
        }
        /* Startup RTC count */
        RTC_Cmd(ENABLE);
    }
}
stc_rtc_date_t pstcRtcDate;
stc_rtc_time_t pstcRtcTime;
/**
 *******************************************************************************
 ** \brief  读取当前日期与时间到全局变量
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   结果存 pstcRtcDate/pstcRtcTime（十进制格式），供显示/计费取用
 ******************************************************************************/
void getrtctime(void)
{
	RTC_GetDate(RTC_DATA_FMT_DEC,&pstcRtcDate);
	RTC_GetTime(RTC_DATA_FMT_DEC, &pstcRtcTime);
}
