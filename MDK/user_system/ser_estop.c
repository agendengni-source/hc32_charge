/**
 *******************************************************************************
 ** \brief  急停按钮消抖状态机
 **
 ** \file    ser_estop.c
 ** \version V1.0
 **
 ** \note    三态消抖（1ms 周期推进，ESTOP_task 调用）：
 **          ESTOP_RELEASED      检测到按下 -> 记时间戳，进 ESTOP_PRESSED
 **          ESTOP_PRESSED       按下持续超过 CONFIRM_TIME(20ms) 才确认；
 **                              提前松开视为抖动，回到释放态
 **          ESTOP_WAIT_RELEASE  已确认：持续按下返回 1，采到松开回到释放态
 **          结果由 app_charge.c 的 stoptask() 消费：按下置故障，释放退出急停
 ******************************************************************************/
#include "ser_estop.h"
#include "drive_emstop.h"
#include "ser_systick.h"
ESTOP_FILTER estop_key;

/* 每次调用推进一次状态机，只对按下做消抖。 */
/* 推进一次急停按下消抖：记录按下时刻、等待确认、持续按下时返回 1。 */
uint8_t Key_state(void)
{
	/* 按下、确认中、释放各路径统一返回：未确认按下一律视为 0。 */
	volatile uint64_t sysTime = 0;
	switch(estop_key.state)
	{
		case ESTOP_RELEASED:
			/* 首次检测到按下，记录消抖起点。 */
			if(KEYPRESS)
			{
				estop_key.state=ESTOP_PRESSED;
				estop_key.release_tick=get_systicktime();
			}
			break;
		case ESTOP_PRESSED:
			/* 按下超过阈值才确认，提前松开视为抖动。 */
			if(KEYPRESS)
			{
				sysTime=get_systicktime();
				if(sysTime-estop_key.release_tick>CONFIRM_TIME)
				{
					estop_key.state=ESTOP_WAIT_RELEASE;

				}
			}
			else
			{
				estop_key.state=ESTOP_RELEASED;
			}
			break;
		case ESTOP_WAIT_RELEASE:
			/* 按下已确认，持续按下返回 1，采到松开就回到释放态。 */
			if(KEYPRESS==0)
			{
				estop_key.state=ESTOP_RELEASED;
			}
			else
			{
				return 1;
			}

			break;
		default:
			estop_key.state=ESTOP_RELEASED;
			break;
	}
	return 0;
}
