/**
 *******************************************************************************
 ** \brief  系统节拍与任务调度器（裸机时间片轮询框架）
 **
 ** \file    ser_systick.c
 ** \version V1.0
 **
 ** \note    架构：SysTick 1ms 中断（TaskScheduleCb）只做倒计时和置标志，
 **          主循环 TaskHandle() 逐个执行到期任务——避免中断里跑重活
 **          任务表 tasknum[] 集中管理所有周期任务（见下表）：
 **          ESTOP_task    1ms    急停消抖+蜂鸣器
 **          RCMU_task     3ms    漏电检测（5次消抖≈15ms确认）
 **          HDC1080_task  200ms  桩内温湿度+过温判决
 **          CHARGE_task   100ms  充电业务状态机
 **          RTC_task      200ms  刷卡帧解析（历史命名，未读RTC）
 **          NTC_task      500ms  漏保端子温度+BL0939参数刷新
 **          LED_task      1000ms 灯语
 **          SCREEN_task   1000ms 串口屏
 ******************************************************************************/
#include "ser_systick.h"
#include "drive_rscard.h"
#include "s_rscard.h"
#include "ser_screenshow.h"
#include "ser_led.h"
#include "drive_ntc.h"
#include "stdio.h"
#include "drive_bl0939.h"
#include "app_charge.h"
#include "drive_beep.h"
#include "drive_gdw25q16.h"
#include "drive_rcmu101.h"
#include "drive_hdc1080.h"

/* 过温阈值：桩内环境用HDC1080，漏保端子用NTC；回差用于自动恢复。 */
#define PILE_TEMP_FAULT    60.0f   /* 桩内环境过温停机阈值(℃) */
#define PILE_TEMP_RECOVER  50.0f   /* 桩内环境恢复阈值(℃) */
#define NTC_TEMP_FAULT     85.0f   /* 漏保端子过温停机阈值(℃) */
#define NTC_TEMP_RECOVER   75.0f   /* 漏保端子恢复阈值(℃) */
/* 1 ms 计数由 SysTick 中断更新，供业务任务计算经过时间。 */
volatile uint32_t systicktime=0;
/* LED 慢闪计时：[0]由 SysTick 递增，[1]为翻转周期(ms)，led_slowflash() 比较使用 */
volatile uint32_t led_slowtime[2]={0,500};
/* 蜂鸣器毫秒时基（drive_beep.c 使用） */
volatile uint32_t beeptime=0;

static void NTC_task(void);
static void ESTOP_task(void);
static void CHARGE_task(void);
static void RTC_task(void);
static void LED_task(void);
static void SCREEN_task(void);
static void RCMU_task(void);
static void HDC1080_task(void);
static void bl0939task(void);
/* 按任务表实际长度确定遍历上限，合法下标为 0 至 tasknumbuf - 1。 */
#define tasknumbuf (sizeof(tasknum)/sizeof(tasknum[0]))
/* 任务表各列依次为待执行标志、首次倒计时、重装周期（ms）和函数指针。 */
task tasknum[]={
	{0,1,1,ESTOP_task},
	{0,3,3,RCMU_task},
	{0,200,200,HDC1080_task},
	{0,100,100,CHARGE_task},
	{0,200,200,RTC_task},
	{0,500,500,NTC_task},
	{0,1000,1000,LED_task},
	{0,1000,1000,SCREEN_task},

};
/**
 *******************************************************************************
 ** \brief  任务倒计时与标志置位（SysTick 中断内调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   每 1ms 将各任务倒计时减一；到 0 时置标志并重装周期。
 **         只置标志不执行任务，重活留给主循环
 ******************************************************************************/
void TaskScheduleCb(void)
{
	for(uint8_t i=0;i<tasknumbuf;i++)
	{
		if(tasknum[i].tasktime)
		{
		tasknum[i].tasktime--;
		if(tasknum[i].tasktime==0)
		{
			tasknum[i].taskflag=1;
			tasknum[i].tasktime=tasknum[i].tasktimeARR;
		}
		}
	}
}
//系统时钟
/**
 *******************************************************************************
 ** \brief  启动 SysTick 1ms 节拍
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   SysTick_Init(1000) 表示 1kHz 中断频率；调用前需 apptaskinit()
 ******************************************************************************/
void s_SystickInit(void)
{

	SysTick_Init(1000);
}
static void (*hc_pTaskScheduleFunc)(void);
/* 注册任务调度回调，供 SysTick_Handler() 调用。 */
void apptaskinit(void)
{
	hc_pTaskScheduleFunc=TaskScheduleCb;

}
/**
 *******************************************************************************
 ** \brief  SysTick 中断服务函数（1ms）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   职责：①系统毫秒计数 ②LED/蜂鸣时基 ③任务倒计时调度
 **          ④读卡帧超时判定——9600bps 下字节间隔约1ms，
 **          超 5ms 无新字节即认为一帧接收完成，置 u3overtimeflag 通知解析任务
 ******************************************************************************/
void SysTick_Handler(void)
{
	SysTick_IncTick();
	systicktime++;
	led_slowtime[0]++;
	beeptime++;
	hc_pTaskScheduleFunc();
	/* 只有串口接收进行中才累计间隔；超过 5 个节拍时通知解析任务。 */
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
/**
 *******************************************************************************
 ** \brief  忙等式毫秒延时
 **
 ** \param  [in] time  延时毫秒数
 **
 ** \retval None
 ** \note   基于 get_systicktime() 差值忙等，等待期间整个任务系统停摆，
 **          只允许在初始化/自检等必要场合短时使用，业务循环中禁用
 ******************************************************************************/
void SysTick_Delayms(uint32_t time)
{
	uint32_t nowtime=get_systicktime();
	while(get_systicktime()-nowtime<time)
	{
	}
}
/**
 *******************************************************************************
 ** \brief  获取系统上电以来的毫秒计数
 **
 ** \param  [in] None
 **
 ** \retval uint32_t  毫秒计数（约 49.7 天回绕一次，差值比较法天然安全）
 ******************************************************************************/
uint32_t get_systicktime(void)
{
	return systicktime;
}
/**
 *******************************************************************************
 ** \brief  急停处理任务（1ms 周期）+ 蜂鸣器推进
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void ESTOP_task(void)
{
	stoptask();
	beeptask();
}
/**
 *******************************************************************************
 ** \brief  充电业务状态机任务（100ms 周期）
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void CHARGE_task(void)
{

	App_ChargeTask();

}
/**
 *******************************************************************************
 ** \brief  漏电检测任务（3ms 周期，含 5 次消抖）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   连续 5 次采到 VO 有效（约 15ms）才确认漏电，防继电器动作毛刺误报；
 **          确认后置故障状态与故障原因（漏电 → 红灯闪3次/屏显75）；
 **          注意只置 Charging_issue，屏幕故障码由 APP_FAULT 分支统一映射
 ******************************************************************************/
void RCMU_task(void)
{

	static uint8_t high_cnt=0;
	if(REAIL_ERROR())
	{
		high_cnt++;
		if(high_cnt>=5)   /* 连续5次(约15ms)为高才确认，防继电器动作毛刺误报 */
		{
			high_cnt=5;
			char_state = APP_FAULT;
			Charging_issue = FAULT_LEAKAGE;
		}
	}
	else
	{
		high_cnt=0;
	}
}

uint32_t ID=0;
uint8_t card_result;
/**
 *******************************************************************************
 ** \brief  刷卡解析任务（200ms 周期；函数名沿用"RTC"，实际不读RTC）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   调用 compdate() 解析一帧读卡数据并做白名单校验，
 **          结果存 card_result（2=有效白名单卡），由充电状态机查询；
 **          TODO：Flash ID 读取（GD25Q_ReadJEDEC_ID）已注释停用，
 **          任务命名与内容不符，后续应更名
 ******************************************************************************/
void RTC_task(void)
{
//	ID=GD25Q_ReadJEDEC_ID();
	card_result=compdate();

}
volatile float r;
/**
 *******************************************************************************
 ** \brief  NTC 测温 + 计量刷新任务（500ms 周期）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   ①读漏保端子温度 r（B值法100次均值）并刷新屏幕 temp 控件
 **          ②刷新 BL0939 全部电参数缓存 + 屏幕 vol 控件（电压）
 **          ③端子温度 ≥85℃ 置过温故障；恢复判断统一在 HDC1080_task
 ******************************************************************************/
void NTC_task(void)
{
		r=NTC_NCT_RValue();
//		printf("temp.txt=\"%.2f\"\xff\xff\xff",r);
		read_bl0903alldate();
		printf("vol.txt=\"%.2f\"\xff\xff\xff",blo939_value.volate_value);
			if(r>=NTC_TEMP_FAULT)
	{
		char_state = APP_FAULT;
		Charging_issue = FAULT_OVER_TEMPERATURE;
	}
}
/**
 *******************************************************************************
 ** \brief  桩内温湿度检测与过温判决任务（200ms 周期）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   两阶段轮询 HDC1080（本次触发/下次读取交替）；
 **          环境温度 ≥60℃ 置过温故障；
 **          恢复条件：桩内温度 ≤50℃ 且漏保端子温度 ≤75℃，
 **          且当前故障原因正是过温（防误恢复其它故障）；
 **          湿度只采集不参与判决（国标无强制湿度停机要求）
 ******************************************************************************/
void HDC1080_task(void)
{
	HDC1080_Data_t pstOut;
DRV_HDC1080_Poll(&pstOut);
	printf("temp.txt=\"%.2f\"\xff\xff\xff",pstOut.fTemp);
	if(pstOut.fTemp>=PILE_TEMP_FAULT)
	{
		char_state = APP_FAULT;
    Charging_issue = FAULT_OVER_TEMPERATURE;
	}
		/* 过温自动恢复：桩内环境和漏保端子都回落到恢复阈值以下 */
	else if(char_state==APP_FAULT && Charging_issue==FAULT_OVER_TEMPERATURE&& pstOut.fTemp<=PILE_TEMP_RECOVER && r<=NTC_TEMP_RECOVER)
	{
		char_state = APP_IDLE;
	}

}
/**
 *******************************************************************************
 ** \brief  灯语任务（1000ms 周期）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   TODO：灯语内部按 300ms 半拍设计（Alternat_flashtime），
 **          任务周期 1000ms 会把节奏放慢约 3 倍，如需标准节奏
 **          应缩短本任务周期或改用独立计时
 ******************************************************************************/
void LED_task(void)
{
	led_Language();

}
/**
 *******************************************************************************
 ** \brief  串口屏刷新任务（1000ms 周期）
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void SCREEN_task(void)
{
	ScreenState_Show();
}

/**
 *******************************************************************************
 ** \brief  主循环任务执行器：逐个执行已到期任务
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   标志清零后立即执行，同一任务多次到期会合并为一次执行；
 **          在 main 的死循环中反复调用
 ******************************************************************************/
void TaskHandle(void)
{
	for(uint8_t i=0;i<tasknumbuf;i++)
	{
	/* 标志为 1 表示任务待执行；同一任务多次到期可合并为一次执行。 */
	if(tasknum[i].taskflag)
		{
			tasknum[i].taskflag=0;
			tasknum[i].taskfun();
		}
	}
}
void bl0939task(void)
{
	static uint8_t flag=0; 
	if( blo939_value.current_value> 32)
	{
		/*灯的操作*/
		char_state = APP_FAULT;
    Charging_issue = FAULT_OVER_CURRENT;
		screenshow.showfault =SHOW_FAULT_OVERCURRENT;
		return;
	}
	else if( blo939_value.current_value < 1.0f &&  blo939_value.current_value > 0.5f)
	{
		char_state = APP_FAULT;
    Charging_issue = FAULT_LOW_CURRENT;
		screenshow.showfault =SHOW_FAULT_LOW_CURRENT;
	}
	else if( blo939_value.volate_value < 176)
	{
		/*灯的操作*/
	char_state = APP_FAULT;
    Charging_issue = FAULT_UNDER_VOLTAGE;
		screenshow.showfault =SHOW_FAULT_UNDERVOLT;
		/*如果松开就自动进入空闲*/
		/*没有松开就不执行下面状态*/
		return;
	}
	else if(blo939_value.volate_value > 264)
	{
		/*灯的操作*/
		char_state = APP_FAULT;
    Charging_issue =  FAULT_OVER_VOLTAGE;
		screenshow.showfault =SHOW_FAULT_OVERVOLT;
		/*没有松开就不执行下面状态*/
		return;
	}
}