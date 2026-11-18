#include "main.h"
#include "drive_led.h"
#include "drive_beep.h"
#include "drive_emstop.h"
#include "drive_clkcfg.h"
#include "drive_relay.h"
#include "drive_uart.h"
#include "ser_screenshow.h"
#include "ser_systick.h"
#include "drive_rscard.h"
#include "s_rscard.h"
#include "ser_led.h"
#include "stdio.h"
#include "drive_gdw25q16.h"
#include "drive_ntc.h"
#include "drive_bl0939.h"
#include "drive_cp.h"
#include "drive_hdc1080.h"
#include "drive_rcmu101.h"
#include "drive_dswb04.h"
#include "ser_cp.h"
#include "drive_rtc.h"
float r=0;
uint32_t ID=0;
int32_t main(void)
  {
	uint8_t emergency_now;
	uint8_t emergency_last=0;
	uint8_t card_result;
	LL_PERIPH_WE(LL_PERIPH_ALL);
	drive_LedInit();
	drive_BeepInit();
	drive_ClkcfgInit();
//	drive_IntCfg();
	drive_u3IntCfg();
	drive_UartInit();
	drive_RscardInit();
	drive_EmstopInit();
	drive_NtcInit();
	drive_Bl0939Init();
	 drive_Gd25q16Init();
	drive_RelayInit();
	drive_cpInit();
	DRV_HDC1080_Config();
		drive_Rcmu101Init();
		drive_Dswb04Init();
		drive_RtcInit();
	LL_PERIPH_WP(LL_PERIPH_ALL);
	 s_SystickInit();
	ID=GD25Q_ReadJEDEC_ID();
//	LRealyON();
//	NRealyON();
	screenshow.top_state=Normal;
	screenshow.showfault = SHOW_NO_Fault;
// Cp_SetCompareValue(2500);
	car_state=Car_NCON;
	int_state=NCT_internet;
	 HDC1080_Data_t hdcdate;
	 
	for (;;)
	{   
		getrtctime();
//		 DRV_HDC1080_Poll(&hdcdate);
		DDL_DelayMS(100);
		Ser_CPcheck();
		card_result=compdate();
		emergency_now=(GPIO_ReadInputPins(EMSTOP_PORT, EMSTOP_PIN) == PIN_SET);
		if (emergency_now)
		{
			LRealyOFF();
			NRealyOFF();
			screenshow.top_state=	Malfunction;
			screenshow.showfault=SHOW_FAULT_EMERGENCY;
			car_state=Car_Issue;
			Charging_issue=FAULT_EMERGENCY_STOP;
		}
		else if(emergency_last)
			{
				   /* 显示联调：仅在急停刚释放时回待机，丢弃本轮刷卡事件。 */
				  if (screenshow.top_state == Malfunction &&
                screenshow.showfault == SHOW_FAULT_EMERGENCY)
					{
			screenshow.top_state=Normal;
			screenshow.showfault = SHOW_NO_Fault;
			car_state=Car_NCON;
			int_state=NCT_internet;
					}
		}
		else if(card_result==2&&screenshow.top_state==Normal)
			{
		car_state=Car_CON;
		carcon_state=charging;
		screenshow.top_state=Charging;
			}
			 emergency_last = emergency_now;
			
		if(led_slowtime[0]>led_slowtime[1])
		{
			r=NTC_NCT_RValue();
			read_bl0903alldate();
			printf("temp.txt=\"%.2f\"\xff\xff\xff",r);
			led_slowtime[0]=0;
		}
		beeptask();
		led_Language();
		ScreenState_Show(screenshow);
	}	
}