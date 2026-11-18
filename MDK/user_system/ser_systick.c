#include "ser_systick.h"
#include "drive_rscard.h"
#include "s_rscard.h"
volatile uint32_t systicktime=0;
volatile uint32_t led_slowtime[2]={0,500};
volatile uint32_t beeptime=0;
//ÏµÍ³Ê±ÖÓ
void s_SystickInit(void)
{
	
	SysTick_Init(1000);
	
	
}
void SysTick_Handler(void)
{
	SysTick_IncTick();
	systicktime++;
	led_slowtime[0]++;
	beeptime++;
	if(u3state.u3flag==1)
	{
	u3state.u3overtime++;
	if(u3state.u3overtime>5)
	{
		u3state.u3flag=0;
		u3state.u3overtimeflag=1;
		u3state.u3overtime=0;
	}
}
}
void SysTick_Delayms(uint32_t time)
{
	uint32_t nowtime=get_systicktime();
	while(get_systicktime()-nowtime>=time)
	{
	}
}
uint32_t get_systicktime(void)
{
	return systicktime;
}