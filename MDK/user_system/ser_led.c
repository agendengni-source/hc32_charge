#include "ser_led.h"
#include "drive_led.h"
#include "ser_systick.h"

FaultCode_t Charging_issue;
 Car_state car_state; 
 INT_State int_state;
 Carcon_state carcon_state;



/*
作用阈：实现灯语逻辑；
更新灯状态至硬件控制；
*/

static void led_slowflash(led_id id)
{
	if(led_slowtime[0]>=led_slowtime[1])
	{
		ledTuen(id);
		led_slowtime[0]=0;
	}
}
uint32_t systick_time=0;
static void led_greenblue(void)
{
	if(get_systicktime()-systick_time>Alternat_flashtime)
	{
	en_pin_state_t led_flag=GPIO_ReadInputPins(LED_GREEN_PORT,LED_GREEN_PIN);
	if(led_flag==SET)
	{
		LedOff(LED_GREEN);
		LedOn(LED_BLUE);
	}
	else
	{
		LedOn(LED_GREEN);
		LedOff(LED_BLUE);
	}
	systick_time=get_systicktime();
}
}
uint32_t nowtime_red=0;
volatile uint8_t i=0;
static void red_Flicker(uint8_t num)
{
	
	uint32_t now = get_systicktime();

    if (num == 0 || num > 13)
        return;

    /* 已经闪完一组：保持灭灯，等待2秒 */
    if (i >= 2 * num)
    {
        if (now - nowtime_red < 2000)
            return;

        i = 0;  // 等待结束，允许开始下一组
    }

		if(now-nowtime_red>=Alternat_flashtime)
		{
			i++;
			ledTuen(LED_RED);
			nowtime_red=now;
		}
}
static void stop_char()
{
	switch(Charging_issue)
	{
		case FAULT_EMERGENCY_STOP:
			LedOn(LED_RED);
		break;
			case FAULT_UNDER_VOLTAGE:
				red_Flicker(1);
			break;
			case FAULT_OVER_VOLTAGE:
				red_Flicker(2);
			break;
			case FAULT_LEAKAGE:
				red_Flicker(3);
			break;
			case FAULT_RESERVED_NETWORK:
				red_Flicker(4);
			break;
			case FAULT_CARD_READER_COMM:
				red_Flicker(5);
			break;
			case FAULT_OVER_CURRENT:
				red_Flicker(6);
			break;
			case FAULT_LOW_CURRENT:
				red_Flicker(7);
			break;
			case FAULT_OVER_TEMPERATURE:
				red_Flicker(8);
			break;
			case FAULT_VEHICLE_S2_SWITCH:
				red_Flicker(9);
			break;
			case FAULT_RELAY_WELDED:
				red_Flicker(10);
			break;
			case FAULT_GUN_SHORT_CIRCUIT:
				red_Flicker(12);
			break;
			case FAULT_DIODE_MISSING:
				red_Flicker(13);
			break;
			case FAULT_GROUND:
				red_Flicker(11);
			break;
			default:
				break;
	}
}
static void no_char(void)
{
	switch(int_state)
	{
			case CT_internet:
				led_slowflash(LED_BLUE);
				break;
			case NCT_internet:
				LedOn(LED_BLUE);
				break;
			default:
				break;
	}
}
static void carcon(void)
{
	switch(carcon_state)
	{
		case Plugged_ncharged:
			LedOff(LED_BLUE);
			LedOn(LED_GREEN);
			break;
			case charging:
				led_greenblue();
			break;
				case compete:
					LedOff(LED_BLUE);
			LedOn(LED_GREEN);
			break;
				default:
					break;
	}
}
void led_Language(void)
{
	 static Car_state last_car = Car_NCON;
   static FaultCode_t last_fault = FAULT_EMERGENCY_STOP;
	    if (car_state != last_car ||
        (car_state == Car_Issue && Charging_issue != last_fault))
    {
        LedOff(LED_RED);                         // 从灭灯开始
        i = 0;                             // 清除旧故障的次数
        nowtime_red = get_systicktime();    // 重新开始计时
    }
		 last_car = car_state;
    last_fault = Charging_issue;
	switch(car_state)
	{
		case Car_NCON:
				 LedOff(LED_RED); 
		 LedOff(LED_GREEN); 
			no_char();
			break;
		case Car_CON:
			LedOff(LED_RED); 
			carcon();
			break;
		case Car_Issue:
		LedOff(LED_BLUE); 
		LedOff(LED_GREEN); 
			 stop_char();
			break;
		default:
			break;
	}
}