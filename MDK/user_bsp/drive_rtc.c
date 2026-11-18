#include "drive_rtc.h"



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
void getrtctime(void)
{
	RTC_GetDate(RTC_DATA_FMT_DEC,&pstcRtcDate);
	RTC_GetTime(RTC_DATA_FMT_DEC, &pstcRtcTime);
}