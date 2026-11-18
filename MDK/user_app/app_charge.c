#include "app_charge.h"
#include "ser_screenshow.h"
#include "s_rscard.h"
#include "ser_led.h"


CHARGE_STATE char_state=APP_IDLE;
void App_ChargeTask(void)
{
	 switch(char_state)
	 {
		 case APP_IDLE:
		car_state=Car_NCON;
		screenshow.top_state=Normal;
		carcon_state=Plugged_ncharged;
		 break;
		 case APP_WAIL_AUTH:
			 if
			 break;
		 case APP_WAIT_READY:
			 break;
		 case APP_CHARGEING:
			 break;
		 case APP_CMOPETED:
			 break;
		 case APP_FAULT:
			 break;
		 default :
			 break;
	 }
}
