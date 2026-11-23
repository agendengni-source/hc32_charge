/**
 *******************************************************************************
 ** \brief  交流充电桩主程序（HC32F460，裸机时间片轮询架构）
 **
 ** \file    main.c
 ** \version V1.0
 **
 ** \note    初始化流程（外设写保护打开 -> 逐模块初始化 -> 写保护关闭）：
 **          LED/蜂鸣 -> 时钟200MHz -> USART3读卡中断 -> USART1屏
 **          -> 读卡模块 -> 急停 -> NTC -> BL0939 -> GD25Q16
 **          -> 继电器 -> CP -> HDC1080 -> RCMU漏电 -> 拨码 -> RTC
 **          随后注册调度回调、启动 1ms SysTick，进入主循环跑任务表
 **          业务架构三层：user_bsp 驱动层 / user_system 服务层 / user_app 应用层
 ******************************************************************************/
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
#include "app_charge.h"
int32_t main(void)
  {

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
	apptaskinit();
	 s_SystickInit();

//	screenshow.top_state=Normal;
//	screenshow.showfault = SHOW_NO_Fault;
// Cp_SetCompareValue(2500);
//	car_state=Car_NCON;
//	int_state=NCT_internet;
	HDC1080_Data_t hdcdate;
	/* TODO：上电漏电模块自检被注释（自检依赖的 Self_check 当前
	   在 APP_WAIT_READY 里也处于注释状态）——上线前必须恢复其一 */
//	uint8_t a=Self_check();
//	while(a)
//	{
//	}
	for (;;)
	{
		TaskHandle();
//		getrtctime();
//		 DRV_HDC1080_Poll(&hdcdate);
//		DDL_DelayMS(100);

	}
}
