#include "ser_cp.h"
#include "drive_cp.h"
#include "drive_relay.h"
#include "ser_screenshow.h"
#include "s_rscard.h"
#include "ser_led.h"
#include "ser_systick.h"
#include "app_charge.h"


CP_STATE cp_state=CP_UNKNOWN;
CP_STATE lastcp_state=CP_UNKNOWN;
static uint32_t s_tick=0;
volatile uint8_t cp_negative_ok=0; 
static CP_STATE CP_GetState(void)
{
	uint32_t voltage_mv = Cp_voltage.PB0VALUE;
	
	if(11400<=voltage_mv&&voltage_mv<=12000)
	{
		return CP_DISCONNECTED;
	}
	else if(8500<=voltage_mv&&voltage_mv<=9400)
	{
		return CP_CONNECTED;
	}
	else if(5600<=voltage_mv&&voltage_mv<=6400)
	{
		return CP_READY;
	}
	else
	{
		return CP_UNKNOWN;
	}
	
}
static void check(void)
{
	 int32_t voltage_mv = Cp_voltage.PB1VALUE;
	if(-12000<=voltage_mv&&voltage_mv<=-11400)
	{
		cp_negative_ok=1;
	}
	else
	{
		cp_negative_ok=0;
	}
}
void Ser_CPcheck(void)
{
	uint32_t now=get_systicktime();
	if(now-s_tick<10)
	{
		return;
	}
	s_tick=now;

	/* 每10ms更新一次检测结果，当前不执行充电控制 */
	lastcp_state=cp_state;     /* 保存上一次检测的状态 */
	cp_state=CP_GetState();   /* PB0：判断当前车辆状态 */
	check();	/* PB1：更新负半周检查结果 */
	switch(cp_state)
	{
		case CP_UNKNOWN:
			char_state=APP_IDLE;
			break;
		case CP_DISCONNECTED:
			char_state=APP_WAIL_AUTH;
			break;
		case  CP_CONNECTED:
			break;
			case CP_READY:
				
			break;
		default:
			break;
	}
}
