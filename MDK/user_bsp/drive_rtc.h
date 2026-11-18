#ifndef __DRIVE_RTC_H
#define __DRIVE_RTC_H
#include "hc32_ll.h"


void drive_RtcInit(void);
void getrtctime(void);
extern stc_rtc_date_t pstcRtcDate;
extern stc_rtc_time_t pstcRtcTime;
#endif