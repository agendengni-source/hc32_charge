/**
 *******************************************************************************
 ** \brief  串口屏显示服务（淘晶驰 TJC 系列，USART1 驱动）
 **
 ** \file    ser_screenshow.c
 ** \version V1.0
 **
 ** \note    刷新模型（差量刷新，避免每周期重发全部内容）：
 **          - 页面变化：发送切页指令 + 该页初始内容
 **          - 故障页内原因变化：更新标题/提示/故障码
 **          - 数据页到刷新间隔：更新电流/时间/电量/费用等控件
 **          指令格式：控件名="内容"\xff\xff\xff（3 个 0xFF 结束符）
 **          printf 为阻塞发送，指令越少越好
 ******************************************************************************/
#include "ser_screenshow.h"
#include "ser_systick.h"
#include "drive_bl0939.h"
#include "stdio.h"
#include "ser_blo939.h"
/* 业务层写入目标页面与故障原因；显示层记住上次发送的页面和故障。 */
ScreenState screenshow;
charge_state laststate= ScreenInvalid;
ScreenShowFaultSubState latshowfault=SHOW_NO_Fault;
//每次执行屏幕处理：

//    如果是第一次显示，或者页面发生变化：
//        切换到目标页面
//        设置该页的初始内容
//        如果是故障页，显示故障原因
//        记录已经显示的页面

//    如果仍在故障页，但故障类型发生变化：
//        更新故障原因

//    如果到了数据刷新时间：
//        更新当前页面需要的数据
/**
 *******************************************************************************
 ** \brief  待机页（main1）：首进切页并填充静态信息
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   只在页面切换时发送一次：二维码、设备号(99999 占位)、
 **          版本(00-20)、联网状态(离线)、信号(30 占位)
 ******************************************************************************/
void normaltask(void)
{
	if(laststate !=screenshow.top_state)
	{
	printf("page main1\xff\xff\xfftitle.txt=\"充电桩\"\xFF\xFF\xFF");
	printf("qr0.txt=\"https://qr.alipay.com/fkx14266lvcazmu75yg9q6e\"\xFF\xFF\xFF");


	printf("dim=100\xFF\xFF\xFF");
	printf("devNo.txt=\"99999\"\xff\xff\xff");
	printf("ver.txt=\"00-20\"\xff\xff\xff");
	printf("login.txt=\"离线\"\xff\xff\xff");
	printf("sig.txt=\"30\"\xff\xff\xff");
	laststate=screenshow.top_state;
	}
}
uint32_t CHARINGTIME=0;
uint32_t CHATIME=0;
uint32_t CHARgedTIME=0;
/**
 *******************************************************************************
 ** \brief  充电页（charging）：切页初始化 + 每 1s 刷新充电数据
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   切页时把 CHATIME 回拨 1s 使首轮立即刷新；
 **          数据来源：g_stat（统计层）+ blo939_value（计量缓存）；
 **          屏幕控件：time/current/energy/money
 ******************************************************************************/
void chargingtask(void)
{

	if(laststate !=screenshow.top_state)
	{
	printf("page charging\xff\xff\xfftitle.txt=\"充电桩\"\xFF\xFF\xFF");
	printf("qr0.txt=\"https://qr.alipay.com/fkx14266lvcazmu75yg9q6e\"\xFF\xFF\xFF");
	CHARINGTIME=get_systicktime();
	CHATIME=get_systicktime()-1000;
	laststate=screenshow.top_state;
	}
	/* 间隔计时用于限频；数据取自计量缓存和当前充电统计。 */
	if(get_systicktime()-CHATIME>=1000)
	{
	uint32_t s=g_stat.seconds;
	CHATIME=get_systicktime();
	printf("gunInfo.txt=\"A枪充电中\"\xFF\xFF\xFF");
		printf("time.txt=\"充电时间:%02lu:%02lu:%02lu\"\xFF\xFF\xFF",(unsigned long)(s/3600),(unsigned long)((s%3600U)/60U),(unsigned long)(s%60U));
		printf("current.txt=\"充电电流:%.2f\"\xFF\xFF\xFF",blo939_value.current_value);
		printf("energy.txt=\"充电用量:%.3f kWh\"\xff\xff\xff", (double)g_stat.energy);
		printf("money.txt=\"充电费用:%.2f 元\"\xff\xff\xff", (double)g_stat.money);

	}
}
/**
 *******************************************************************************
 ** \brief  完成页（over）：显示本次充电最终结果
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   只在切页时发送一次；数据为离开充电状态时结算好的 g_stat 快照
 ******************************************************************************/
void chargedtask(void)
{
	uint32_t s = g_stat.seconds;
	if(laststate !=screenshow.top_state)
	{
	printf("page over\xff\xff\xfftitle.txt=\"充电桩\"\xFF\xFF\xFF");
	CHARgedTIME=get_systicktime();
	printf("gunInfo.txt=\"充电完成\"\xFF\xFF\xFF");
		printf("time.txt=\"充电总时间:%02lu:%02lu:%02lu\"\xff\xff\xff",(unsigned long)(s/3600U),(unsigned long)((s%3600U)/60U),(unsigned long)(s%60U));
		printf("energy.txt=\"充电总量:%.3f kWh\"\xff\xff\xff", (double)g_stat.energy);
	printf("money.txt=\"充电总费用:%.2f 元\"\xff\xff\xff", (double)g_stat.money);
	laststate=screenshow.top_state;
	}


}
/* 把故障枚举转换为屏幕故障码；故障码变化时才发送。 */
static void Malfunctionshiw(void)
{
	if(latshowfault!=screenshow.showfault)
	{
	switch(screenshow.showfault)
	{
		case SHOW_Fault_S2:
			printf("faultCode.txt=\"故障码:66\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_RELAY:
			printf("faultCode.txt=\"故障码:67\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_EMERGENCY:
			printf("faultCode.txt=\"故障码:72\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_OVERTEMP:
			printf("faultCode.txt=\"故障码:74\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_LEAKAGE:
			printf("faultCode.txt=\"故障码:75\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_LOW_CURRENT:
			printf("faultCode.txt=\"故障码:76\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_OVERCURRENT:
			printf("faultCode.txt=\"故障码:78\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_UNDERVOLT:
			printf("faultCode.txt=\"故障码:7B\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_OVERVOLT:
			printf("faultCode.txt=\"故障码:7D\"\xFF\xFF\xFF");
			break;
		case SHOW_FAULT_NETWORK:
			printf("faultCode.txt=\"故障码:8A\"\xFF\xFF\xFF");
			break;
		default :
			break;

	}
	latshowfault=screenshow.showfault;
}
}
/**
 *******************************************************************************
 ** \brief  故障页（waring）：切页 + 按原因更新标题/提示/故障码
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   首进故障页时强制把 latshowfault 置为"无故障"，
 **          确保本次故障原因一定被刷新一次；
 **          标题区分急停/过温/漏电三类，其余落到通用"设备故障"提示
 ******************************************************************************/
void Malfunctiontask(void)
{
	if(laststate !=screenshow.top_state)
	{
		printf("page waring\xFF\xFF\xFF");
		latshowfault=SHOW_NO_Fault;
		laststate=screenshow.top_state;

	}
	if(latshowfault !=screenshow.showfault)
	{
		  switch (screenshow.showfault)
        {
            case SHOW_FAULT_EMERGENCY:
                printf("faultTitle.txt=\"急停故障\"\xFF\xFF\xFF");
                printf("fInfo1.txt=\"请旋转释放急停按钮\"\xFF\xFF\xFF");
                break;

            case SHOW_FAULT_OVERTEMP:
                printf("faultTitle.txt=\"过温故障\"\xFF\xFF\xFF");
                printf("fInfo1.txt=\"请等待温度恢复\"\xFF\xFF\xFF");
                break;

						case SHOW_FAULT_LEAKAGE:
	printf("faultTitle.txt=\"漏电故障\"\xFF\xFF\xFF");
	printf("fInfo1.txt=\"请断电检查后再试\"\xFF\xFF\xFF");
	break;
            default:
                printf("faultTitle.txt=\"设备故障\"\xFF\xFF\xFF");
                printf("fInfo1.txt=\"请根据故障码检查\"\xFF\xFF\xFF");
                break;
	}
				Malfunctionshiw();

}
	}

/**
 *******************************************************************************
 ** \brief  屏幕刷新总入口（SCREEN_task 1s 周期调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   按 screenshow.top_state 分发；业务层只写状态，显示细节全部收口在本层
 ******************************************************************************/
void ScreenState_Show(void)
{
	switch(screenshow.top_state)
	{
		case Normal:
			normaltask();
			break;
		case Charging:
			 chargingtask();
			break;
		case Charged:
			chargedtask();
			break;
		case Malfunction:
			Malfunctiontask();
			break;
		default:
			break;
	}
}
