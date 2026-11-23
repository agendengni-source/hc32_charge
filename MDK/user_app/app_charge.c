/**
 *******************************************************************************
 ** \brief  充电业务状态机（应用层核心）
 **
 ** \file    app_charge.c
 ** \version V1.0
 **
 ** \note    状态流转图（100ms 周期推进，CHARGE_task 调用）：
 **          APP_IDLE      待机：无车/未刷卡，继电器断开
 **          APP_WAIL_AUTH 已插枪等刷卡：白名单卡通过 -> 进入等待就绪
 **          APP_WAIT_READY 等就绪：CP=6V 且二极管检查通过 -> 漏电自检
 **                         -> Stat_Start 记计量起点 -> 进入充电
 **          APP_CHARGEING  充电中：L/N 继电器合闸，每 1s 刷新电量/费用统计
 **          APP_CMOPETED   完成：分闸、显示总电量费用，5s 后回待机
 **          APP_FAULT      故障：分闸 + 灯语/故障码显示，按原因不同自动恢复
 **          离场结算：由 last_state 检测"刚离开充电态"，补一次 Stat_Update
 **          （覆盖正常完成与故障两条路径，保证完成页数据完整）
 ******************************************************************************/
#include "app_charge.h"
#include "ser_screenshow.h"
#include "s_rscard.h"
#include "ser_led.h"
#include "ser_cp.h"
#include "drive_relay.h"
#include "drive_emstop.h"
#include "ser_systick.h"
#include "ser_estop.h"
#include "drive_bl0939.h"
#include "ser_blo939.h"
static uint32_t charge_start_tick = 0;   /* 合闸时刻：低流保护启动宽限基准 */
/* 刷卡任务最近一次的解析结果；2 表示有效白名单卡。 */
extern uint8_t card_result;
/* 充电业务状态，供本模块决定继电器、灯语和屏幕页面。 */
CHARGE_STATE char_state=APP_IDLE;
/* 当前流程的刷卡标志：有效卡置 1，进入充电分支前清零。 */
uint8_t charge_flag=0;
/* 说明：此处原有的 PRICE_PER_KWH/METER_K 宏已删除（与 ser_blo939.c 重复定义，
   避免改电价/校准系数时改一处漏一处），统一以 ser_blo939.c 中的定义为准 */


/* 读取消抖后的急停结果。按下时置故障及原因；释放时只尝试退出急停故障。 */
void stoptask(void)
{
	uint8_t emergency_now;
	static uint8_t emergency_last=0;
	emergency_now=Key_state();
		if (emergency_now)
		{
			char_state = APP_FAULT;
       Charging_issue = FAULT_EMERGENCY_STOP;
		}
		else if(emergency_last)
			{
				   /* 显示联调：仅在急停刚释放时回待机，丢弃本轮刷卡事件。 */
				  if ((char_state == APP_FAULT) && (Charging_issue == FAULT_EMERGENCY_STOP))
					{
						char_state = APP_IDLE;
					}
		}
			else if( (char_state == APP_CHARGEING )&&(blo939_value.current_value> 32))
	{
		/*灯的操作*/
		char_state = APP_FAULT;
    Charging_issue = FAULT_OVER_CURRENT;
		screenshow.showfault =SHOW_FAULT_OVERCURRENT;
	}
	else if( (char_state == APP_CHARGEING) &&(blo939_value.current_value < 1.0f) && ( blo939_value.current_value > 0.5f))
	{
		char_state = APP_FAULT;
    Charging_issue = FAULT_LOW_CURRENT;
		screenshow.showfault =SHOW_FAULT_LOW_CURRENT;
	}
	else if( (char_state == APP_CHARGEING) &&(blo939_value.volate_value < 176))
	{
		/*灯的操作*/
	char_state = APP_FAULT;
    Charging_issue = FAULT_UNDER_VOLTAGE;
		screenshow.showfault =SHOW_FAULT_UNDERVOLT;
		/*如果松开就自动进入空闲*/
		/*没有松开就不执行下面状态*/
	}
	else if((char_state == APP_CHARGEING) &&(blo939_value.volate_value > 264))
	{
		/*灯的操作*/
		char_state = APP_FAULT;
    Charging_issue =  FAULT_OVER_VOLTAGE;
		screenshow.showfault =SHOW_FAULT_OVERVOLT;
		/*没有松开就不执行下面状态*/
	}

			
			 emergency_last = emergency_now;

}
/* CP 正电压分类发生变化时，结合当前业务状态决定是否切换。 */
static void App_HandleCpState(void)
{
	switch(cp_state)
	{
		case CP_UNKNOWN:
			/* 无法识别 CP 电压时先进入故障状态。 */
//			char_state=APP_FAULT;
			break;
		case CP_DISCONNECTED://12v
			  /* 当前实现将充电中拔枪归入结束流程；拔枪本身不能证明车辆已充满。 */
			  if (char_state == APP_CHARGEING)//当正在充电时拔枪---充电完成
				{
        char_state = APP_CMOPETED;
				}
				else if(char_state == APP_WAIL_AUTH || char_state == APP_WAIT_READY)//当处于已授权，/车辆已连接时9v-12v 变为空闲态
		{
        char_state = APP_IDLE;
		}
//		else if(char_state == APP_CMOPETED)
//		{
//			char_state = APP_IDLE;
//		}
			break;
		case  CP_CONNECTED://9v
			if(char_state== APP_IDLE)
			{
				char_state=APP_WAIL_AUTH;
			}
			/* 车辆退出就绪时，当前实现同样转到结束流程。 */
			else if(char_state==APP_CHARGEING) //6v-9v 充电完成
			{
				char_state=APP_CMOPETED;
			}
			break;
			case CP_READY:
			break;
		default:
			break;
	}

}
/**
 *******************************************************************************
 ** \brief  充电业务状态机（CHARGE_task 100ms 周期调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   执行顺序：离场结算检查 -> CP 变化处理 -> 按状态分发；
 **          继电器/灯语/屏幕状态在各分支内维护；
 **          故障进入路径有三类：急停(stoptask)、漏电(RCMU_task)、
 **          过温(NTC_task/HDC1080_task)，均只置 char_state+Charging_issue，
 **          故障码到屏幕的映射统一在本状态机 APP_FAULT 分支完成
 ******************************************************************************/
void App_ChargeTask(void)
{
	static uint32_t completed_tick = 0;
	/* 完成页计时标志；离开完成状态后清零，供下次重新计时。 */
	static uint8_t completed_timing = 0;
	/* 记住上一轮业务状态，用于识别"刚离开充电态"。 */
	static CHARGE_STATE last_state = APP_IDLE;
	
	
	
	if (char_state != APP_CMOPETED)
	{
    completed_timing = 0;
	}
	/* 无论因完成还是故障离开充电，都补一次结算，保证完成页数据完整。 */
	if (last_state == APP_CHARGEING && char_state != APP_CHARGEING)
	{
		Stat_Update();
	}
	last_state = char_state;
	/* Ser_CPcheck 返回 1 表示正电压分类有变化，届时才处理 CP 状态切换。 */
	if(Ser_CPcheck() !=0)
	{
		App_HandleCpState();
	}
	 switch(char_state)
	 {
		 case APP_IDLE:
		charge_flag=0;
		car_state=Car_NCON;
		screenshow.top_state=Normal;
		carcon_state=Plugged_ncharged;
		 break;
		 case APP_WAIL_AUTH:
			 /* 等待白名单卡；刷卡通过后转入等待车辆就绪。 */
			 screenshow.top_state=Normal;
			 if(card_result==2)
			 {
				 charge_flag=1;

			 }
			 if(charge_flag)
			 {
				 char_state=APP_WAIT_READY;
			 }
			 break;
		 case APP_WAIT_READY:
			 /* 车辆就绪、刷卡标志和负半周检查均通过后，记录本次计量起点。 */
			 if(cp_state==CP_READY&&charge_flag==1&&cp_negative_ok==1)
			 {
//				 if(Self_check()!=0)   /* 自检失败：按漏电模块故障处理 */
//		{
//			char_state=APP_FAULT;
//			Charging_issue=FAULT_LEAKAGE;
//			break;
//		}
				 charge_flag=0;
				  charge_start_tick = get_systicktime();
				 Stat_Start();
				 char_state=APP_CHARGEING;
			 }
			 break;
		 case APP_CHARGEING:
			 /* 充电状态下输出 L、N 合闸命令；此处未再次校验全部保护条件。 */
			 LRealyON();
		 NRealyON();
		 static uint32_t s_tick=0;
/* 约每秒更新一次充电时长、用电量和费用。 */
if((uint32_t)(get_systicktime()-s_tick)>=1000U)
{ s_tick=get_systicktime();
	Stat_Update();
}
		car_state=Car_CON;
		screenshow.top_state=Charging;
		carcon_state=charging;
			 break;
		 case APP_CMOPETED:
			 /* 首次进入完成分支时记录时间，约 5 秒后将业务状态切回待机。 */
			 if (!completed_timing)
{
    completed_tick = get_systicktime();
    completed_timing = 1;
}
if ((uint32_t)(get_systicktime() - completed_tick) >= 5000U)
{
    char_state = APP_IDLE;
    screenshow.top_state = Normal;
    carcon_state = Plugged_ncharged;
}
			  /* 结束分支关断继电器，并设置本轮完成页与灯语状态。 */
			  LRealyOFF();
				NRealyOFF();
			 	car_state=Car_NCON;
		screenshow.top_state=Charged;
		carcon_state=compete;
			 break;
		 case APP_FAULT:
					/* 先发出断开命令，再设置故障灯和页面；故障码映射收口在此处。 */
					LRealyOFF();
					NRealyOFF();
			 	 	car_state=Car_Issue;
					screenshow.top_state=Malfunction;
		switch(Charging_issue)
{
	case FAULT_EMERGENCY_STOP:
		screenshow.showfault = SHOW_FAULT_EMERGENCY;
		break;
	case FAULT_LEAKAGE:
		screenshow.showfault = SHOW_FAULT_LEAKAGE;
		break;
	case FAULT_OVER_TEMPERATURE:
		screenshow.showfault = SHOW_FAULT_OVERTEMP;
		break;
	default:
//		screenshow.showfault = SHOW_NO_Fault;
		break;
}
		carcon_state=Plugged_ncharged;
		 if (cp_state == CP_DISCONNECTED)
		 {			 /* 出口：拔枪后自动复位 */
            char_state = APP_IDLE;
		 }
			 break;
		 default :
			 break;
	 }
}
