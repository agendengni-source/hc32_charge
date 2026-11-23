/**
 *******************************************************************************
 ** \brief  RCMU101 剩余电流传感器（漏电检测模块）驱动
 **
 ** \file    drive_rcmu101.c
 ** \version V1.0
 **
 ** \note    硬件连接（原理图 U31：RCMU101SM3-6EI-U）：
 **          - PE7 ← VO   漏电/自检指示输出（高电平有效，10k 上拉）
 **          - PE8 → CHK  自检触发（MCU 输出控制，10k 下拉）
 **          - L/N 双线穿过模块内部互感器，检测剩余电流矢量和
 **          判决消抖由服务层 RCMU_task() 完成（连续 5 次≈15ms 为高才确认，
 **          防继电器分合闸瞬间毛刺误报；远快于 30mA AC 档要求的动作时限）
 ******************************************************************************/
#include "drive_rcmu101.h"
#include "ser_systick.h"
#define RCMU_VOUT_PORT GPIO_PORT_E
#define RCMU_VOUT_PIN GPIO_PIN_07
#define RCMU_CHECK_PORT GPIO_PORT_E
#define RCMU_CHECK_PIN GPIO_PIN_08

/**
 *******************************************************************************
 ** \brief  漏电模块 GPIO 初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PE7（VO）：数字输入 + 内部上拉（模块开漏输出时保证可靠读回）
 **         PE8（CHK）：推挽输出，默认低电平（不触发自检）
 ******************************************************************************/
void drive_Rcmu101Init(void)
{
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PullUp = PIN_PU_ON;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_E, GPIO_PIN_07, &stcGpioInit);

    /* PE8 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GPIO_PORT_E, GPIO_PIN_08, &stcGpioInit);
}
/**
 *******************************************************************************
 ** \brief  漏电模块自检（验证传感器与判决链路健康）
 **
 ** \param  [in] None
 **
 ** \retval uint8_t  0:自检通过  1:自检失败
 ** \note   流程：CHK 拉高 → 模拟故障 → 读 VO 应为高 → 拉低 CHK → 结束
 **         每笔充电在 APP_WAIT_READY 合闸前调用一次；自检期间 VO 的高电平
 **         会被 REAIL_ERROR() 的 CHK 状态判断屏蔽，不误报漏电
 ** \note   TODO：手册暂缺，CHK 拉高到 VO 应答的延时未知，当前 3ms 为估计值，
 **         建议放宽到 10ms 并实测确认（两个延时都要改）
 ******************************************************************************/
uint8_t Self_check(void)
{
	uint8_t vout_during_check = 0;
	GPIO_SetPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN);
	SysTick_Delayms(3);
	vout_during_check=(GPIO_ReadInputPins(RCMU_VOUT_PORT,RCMU_VOUT_PIN));
	GPIO_ResetPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN);
		SysTick_Delayms(3);

	if(vout_during_check)
	{
		//自检成功
		return 0;
	}
	return 1;
}
/**
 *******************************************************************************
 ** \brief  读取漏电动作状态（带自检屏蔽）
 **
 ** \param  [in] None
 **
 ** \retval uint8_t  1:VO 有效（疑似漏电动作）  0:正常或自检期间
 ** \note   判断逻辑：VO 为高且 CHK 为低（非自检期）才判为有效；
 **         自检触发期间（CHK=1）VO 的高电平是自检模拟故障，不作为漏电判决
 **         单次采样不做消抖——消抖计数在服务层 RCMU_task() 中完成
 ******************************************************************************/
uint8_t REAIL_ERROR(void)
{
	uint8_t value=0;
	value=GPIO_ReadInputPins(RCMU_VOUT_PORT,RCMU_VOUT_PIN);
	if(value==1)
	{
		if(GPIO_ReadInputPins(RCMU_CHECK_PORT,RCMU_CHECK_PIN))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}
