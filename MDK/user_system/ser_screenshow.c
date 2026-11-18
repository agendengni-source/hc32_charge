#include "ser_screenshow.h"
#include "ser_systick.h"
#include "drive_bl0939.h"
#include "stdio.h"
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
void normaltask(void)
{
	if(laststate !=screenshow.top_state)
	{
	printf("page main1\xff\xff\xfftitle.txt=\"充电桩\"\xFF\xFF\xFF");
	printf("qr0.txt=\"https://qr.alipay.com/fkx14266lvcazmu75yg9q6e\"\xFF\xFF\xFF");
	printf("vol.txt=\"220V\"\xff\xff\xff");
		
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
void chargingtask(void)
{
	
	if(laststate !=screenshow.top_state)
	{
	printf("page charging\xff\xff\xfftitle.txt=\"充电桩\"\xFF\xFF\xFF");
	printf("qr0.txt=\"https://qr.alipay.com/fkx14266lvcazmu75yg9q6e\"\xFF\xFF\xFF");
	
	CHARINGTIME=get_systicktime();
	CHATIME=get_systicktime();
	laststate=screenshow.top_state;
	}
	if(get_systicktime()-CHATIME>500)
	{
	uint32_t seconds =(get_systicktime() - CHARINGTIME) / 1000U;
	printf("gunInfo.txt=\"电压:%.1f V\"\xFF\xFF\xFF", (double)blo939_value.volate_value);
	printf("time.txt=\"%.2lu\"\xFF\xFF\xFF",(unsigned long)seconds);
	printf("current.txt=\"current_value:%.2f\"\xFF\xFF\xFF",blo939_value.current_value);
	printf("energy.txt=\"Power_value:%.2f\"\xFF\xFF\xFF",blo939_value.Power_value);
		printf("money.txt=\"Pulsech_value:%g\"\xFF\xFF\xFF",blo939_value.Pulsecharge_value);
	CHATIME=get_systicktime();
	}
}
void chargedtask(void)
{
	if(laststate !=screenshow.top_state)
	{
	printf("page over\xff\xff\xfftitle.txt=\"充电桩\"\xFF\xFF\xFF");
	CHARgedTIME=get_systicktime();
	printf("gunInfo.txt=\"11111\"\xFF\xFF\xFF");
	printf("time.txt=\"%d\"\xFF\xFF\xFF",(CHARgedTIME-CHARINGTIME)/1000);
	printf("current.txt=\"3333\"\xFF\xFF\xFF");
	printf("energy.txt=\"44444\"\xFF\xFF\xFF");
	printf("money.txt=\"55555\"\xFF\xFF\xFF");
	laststate=screenshow.top_state;
	}

	
}
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

            default:
                printf("faultTitle.txt=\"设备故障\"\xFF\xFF\xFF");
                printf("fInfo1.txt=\"请根据故障码检查\"\xFF\xFF\xFF");
                break;
	}
				Malfunctionshiw();
	
}
	}

void ScreenState_Show(ScreenState screenshow)
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