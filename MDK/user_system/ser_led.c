/**
 *******************************************************************************
 ** \brief  状态指示灯语服务（三色灯语义化显示）
 **
 ** \file    ser_led.c
 ** \version V1.0
 **
 ** \note    灯语总入口 led_Language() 由 LED_task 周期调用：
 **          - Car_NCON：按联网状态蓝灯慢闪/常亮
 **          - Car_CON ：按子状态绿灯常亮/绿蓝交替
 **          - Car_Issue：按 Charging_issue 红灯常亮或闪 N 次报故障码
 **          状态切换或故障原因变化时重置闪烁进度，从头显示新灯语
 **          TODO：本任务周期 1000ms，而灯语按 300ms 半拍设计，
 **          实际闪烁节奏被放慢约 3 倍（故障闪 3 次一组约需 8 秒）
 ******************************************************************************/
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

/* 到达慢闪间隔时翻转指定 LED，并重置该计时。 */
static void led_slowflash(led_id id)
{
	if(led_slowtime[0]>=led_slowtime[1])
	{
		ledTuen(id);
		led_slowtime[0]=0;
	}
}
uint32_t systick_time=0;
/* 按设定间隔在蓝灯和绿灯之间切换，供充电灯语使用。 */
static void led_greenblue(void)
{
	if(get_systicktime()-systick_time>Alternat_flashtime)
	{
	en_pin_state_t led_flag=GPIO_ReadInputPins(LED_GREEN_PORT,LED_GREEN_PIN);
	/* 读绿灯当前电平决定切向：亮则切蓝，灭则切绿，实现交替 */
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
/* 按半拍翻转红灯：两次翻转算一次闪烁，一组结束后暂停再重播。 */
static void red_Flicker(uint8_t num)
{

	uint32_t now = get_systicktime();

    if (num == 0 || num > 13)
        return;

    /* 已经闪完一组：保持灭灯，等待2秒 */
    /* 一组用 2 × num 次翻转完成；达到次数后等待组间间隔。 */
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
/* 把故障码映射为红灯常亮或指定次数的闪烁。 */
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
/* 车辆未连接时，按联网状态让蓝灯慢闪或常亮。 */
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
/* 车辆已连接时，按等待、充电中和完成状态输出对应灯语。 */
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
/**
 *******************************************************************************
 ** \brief  灯语总入口（LED_task 周期调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   先处理"状态/故障原因变化"的重置（灭灯、清闪烁计数、重计时），
 **          再按 car_state 分发到三个灯语分支
 ******************************************************************************/
void led_Language(void)
{
	 static Car_state last_car = Car_NCON;
   static FaultCode_t last_fault = FAULT_EMERGENCY_STOP;
	    /* 进入新状态或换故障原因时，从头显示新灯语。 */
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
