/**
 *******************************************************************************
 ** \brief  CP（控制导引）信号驱动：PWM 生成 + 正负半周电压采样
 **
 ** \file    drive_cp.c
 ** \version V1.0
 **
 ** \note    信号链路：PB13(TMRA1_CH5) 输出 1kHz PWM → CP 驱动芯片 → CP线
 **          → 车端电阻/S2 网络 → 分压后由 ADC2 CH4/CH5 采样
 **          时序参数：TMRA1 时钟 PCLK1(100MHz)/16 = 6.25MHz，
 **          周期 0x186A = 6250 → PWM 频率 1kHz；
 **          CH5 比较值 0x0D03 = 3331 ≈ 53.3% → 国标 32A 电流档
 **          采样机制：CH6/CH7 比较匹配中断在 PWM 周期两个相位点分别触发
 **          ADC2 单次转换——正半周采 CH4（+12/+9/+6V 档），
 **          负半周采 CH5（-12/-9V 档，用于检测二极管是否存在）
 ******************************************************************************/
#include "drive_cp.h"
//static void INT_SRC_TMRA_1_OVF_IrqCallback(void)
//{
//    /* USER CODE BEGIN INT_SRC_TMRA_1_OVF_IrqCallback */
//		TMRA_ClearStatus(CM_TMRA_1, TMRA_FLAG_OVF);
//    /* USER CODE END INT_SRC_TMRA_1_OVF_IrqCallback */
//}
/* 采样相位:  s_u8SamplePhase  1=CH6 触发(采正电平 CH4), 2=CH7 触发(采负电平 CH5), 0=无待读数据 */
static volatile uint8_t  s_u8SamplePhase = 0U;
/**
 *******************************************************************************
 ** \brief  TMRA1 比较匹配中断：按相位启动对应的 ADC2 转换
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   CH6 匹配 → 使能 CH4 采正电平；CH7 匹配 → 使能 CH5 采负电平；
 **         每次只使能一个通道，采完由 ADC2 EOCA 中断取数
 ******************************************************************************/
static void INT_SRC_TMRA_1_CMP_IrqCallback(void)
{
    /* USER CODE BEGIN INT_SRC_TMRA_1_CMP_IrqCallback */
		if(TMRA_GetStatus(CM_TMRA_1, TMRA_FLAG_CMP_CH6)==SET)
		{
			TMRA_ClearStatus(CM_TMRA_1, TMRA_FLAG_CMP_CH6);
			s_u8SamplePhase = 1U;
			ADC_ChCmd(CM_ADC2,  ADC_SEQ_A, ADC_CH4,ENABLE);
			ADC_ChCmd(CM_ADC2,  ADC_SEQ_A, ADC_CH5,DISABLE);
			ADC_Start(CM_ADC2);
		}
		else if(TMRA_GetStatus(CM_TMRA_1, TMRA_FLAG_CMP_CH7)==SET)
		{
			TMRA_ClearStatus(CM_TMRA_1, TMRA_FLAG_CMP_CH7);
			s_u8SamplePhase=2;
			ADC_ChCmd(CM_ADC2,  ADC_SEQ_A, ADC_CH5,ENABLE);
			ADC_ChCmd(CM_ADC2,  ADC_SEQ_A, ADC_CH4,DISABLE);
			ADC_Start(CM_ADC2);
		}

    /* USER CODE END INT_SRC_TMRA_1_CMP_IrqCallback */
}
/* INT_SRC_ADC2_EOCA Callback. */
CP_VOLTAGE Cp_voltage;
/**
 *******************************************************************************
 ** \brief  ADC2 序列A转换完成中断：读取采样值并换算为 CP 电压(mV)
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   换算公式（分压比 47/11，二极管压降 0.7V 补偿）：
 **         正电平：V = adc×3300/4096 × 47/11 + 700 (mV)
 **         负电平：V = -(adc×3300/4096 × 47/11) - 700 (mV，取负得到有符号值)
 **         例：+12V 档 → 引脚约2.64V → 换算回约12000mV
 ******************************************************************************/
static void INT_SRC_ADC2_EOCA_IrqCallback(void)
{
    /* USER CODE BEGIN INT_SRC_ADC2_EOCA_IrqCallback */
	ADC_ClearStatus(CM_ADC2, ADC_FLAG_EOCA);
	if(s_u8SamplePhase==1)
	{
		//(x-0.7)11/47  3.3*vlaue\4096
		Cp_voltage.PB0VALUE=(ADC_GetValue(CM_ADC2,ADC_CH4)*3300/4096)*47/11+700;

	}
	else if(s_u8SamplePhase==2)
	{
		Cp_voltage.PB1VALUE=-(ADC_GetValue(CM_ADC2,ADC_CH5)*3300/4096)*47/11-700;

	}
    /* USER CODE END INT_SRC_ADC2_EOCA_IrqCallback */
}
/**
 *******************************************************************************
 ** \brief  TMRA1 定时器/PWM 配置（CP 载波生成）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   - CH5：PWM 输出脚（PB13），比较值 0x0D03=3331/6250≈53.3% → 32A 档
 **         - CH6：比较值 0x0681=1665（约26.6%），匹配中断触发正电平采样
 **         - CH7：比较值 0x12B6=4790（约76.6%），匹配中断触发负电平采样
 **         两个采样点随占空比联动调整（见 Cp_SetCompareValue）
 ******************************************************************************/
void drive_CpInit(void)
{
	stc_irq_signin_config_t stcIrq;
	stc_tmra_init_t stcTmraInit;
    stc_tmra_pwm_init_t stcPwmInit;
	FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMRA_1, ENABLE);
	  GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_13,GPIO_FUNC_4);//TIMA-1-PWM5
//	  stcIrq.enIntSrc = INT_SRC_TMRA_1_OVF;
//    stcIrq.enIRQn = INT082_IRQn;
//    stcIrq.pfnCallback = &INT_SRC_TMRA_1_OVF_IrqCallback;
//    (void)INTC_IrqSignIn(&stcIrq);
//    /* NVIC config */
//    NVIC_ClearPendingIRQ(INT082_IRQn);
//    NVIC_SetPriority(INT082_IRQn, DDL_IRQ_PRIO_15);
//    NVIC_EnableIRQ(INT082_IRQn);

    /************************* Configure TMRA_1 counter *************************/
    (void)TMRA_StructInit(&stcTmraInit);
    /* Config software count */
    stcTmraInit.sw_count.u8ClockDiv = TMRA_CLK_DIV16;
    stcTmraInit.sw_count.u8CountMode = TMRA_MD_SAWTOOTH;
    stcTmraInit.sw_count.u8CountDir = TMRA_DIR_UP;
    stcTmraInit.u32PeriodValue = 0x186AU;
    (void)TMRA_Init(CM_TMRA_1, &stcTmraInit);

    /* Enable overflow interrupt */
//    TMRA_IntCmd(CM_TMRA_1, TMRA_INT_OVF, ENABLE);

    /************************* Configure TMRA_1_5 CMP ***************************/
    /* Config PWM output */
    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = 0x0D03U;
    stcPwmInit.u16StartPolarity = TMRA_PWM_HOLD;
    stcPwmInit.u16StopPolarity = TMRA_PWM_LOW;
    stcPwmInit.u16CompareMatchPolarity = TMRA_PWM_HIGH;
    stcPwmInit.u16PeriodMatchPolarity = TMRA_PWM_LOW;
    (void)TMRA_PWM_Init(CM_TMRA_1, TMRA_CH5, &stcPwmInit);
    /* PWM pin function set */
    TMRA_SetFunc(CM_TMRA_1, TMRA_CH5, TMRA_FUNC_CMP);
    /* PWM output command */
    TMRA_PWM_OutputCmd(CM_TMRA_1, TMRA_CH5, ENABLE);
		  (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = 0x0681U;
    stcPwmInit.u16StartPolarity = TMRA_PWM_HOLD;
    stcPwmInit.u16StopPolarity = TMRA_PWM_LOW;
    stcPwmInit.u16CompareMatchPolarity = TMRA_PWM_LOW;
    stcPwmInit.u16PeriodMatchPolarity = TMRA_PWM_LOW;
    (void)TMRA_PWM_Init(CM_TMRA_1, TMRA_CH6, &stcPwmInit);
    /* PWM pin function set */
    TMRA_SetFunc(CM_TMRA_1, TMRA_CH6, TMRA_FUNC_CMP);
    /* Enable CMP interrupt */
    TMRA_IntCmd(CM_TMRA_1, TMRA_INT_CMP_CH6, ENABLE);

    /************************* Configure TMRA_1_7 CMP ***************************/
    /* Config PWM output */
    (void)TMRA_PWM_StructInit(&stcPwmInit);
    stcPwmInit.u32CompareValue = 0x12B6U;
    stcPwmInit.u16StartPolarity = TMRA_PWM_HOLD;
    stcPwmInit.u16StopPolarity = TMRA_PWM_LOW;
    stcPwmInit.u16CompareMatchPolarity = TMRA_PWM_LOW;
    stcPwmInit.u16PeriodMatchPolarity = TMRA_PWM_LOW;
    (void)TMRA_PWM_Init(CM_TMRA_1, TMRA_CH7, &stcPwmInit);
    /* PWM pin function set */
    TMRA_SetFunc(CM_TMRA_1, TMRA_CH7, TMRA_FUNC_CMP);
    /* Enable CMP interrupt */
    TMRA_IntCmd(CM_TMRA_1, TMRA_INT_CMP_CH7, ENABLE);

    /* Start timerA */
    TMRA_Start(CM_TMRA_1);
}
/**
 *******************************************************************************
 ** \brief  CP 相关中断注册（TMRA1 比较匹配 + ADC2 序列A完成）
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
static void Cp_IntCfg(void)
{
	stc_irq_signin_config_t stcIrq;
	   stcIrq.enIntSrc = INT_SRC_TMRA_1_CMP;
    stcIrq.enIRQn = INT083_IRQn;
    stcIrq.pfnCallback = &INT_SRC_TMRA_1_CMP_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT083_IRQn);
    NVIC_SetPriority(INT083_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT083_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_ADC2_EOCA;
    stcIrq.enIRQn = INT116_IRQn;
    stcIrq.pfnCallback = &INT_SRC_ADC2_EOCA_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT116_IRQn);
    NVIC_SetPriority(INT116_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT116_IRQn);
}
/**
 *******************************************************************************
 ** \brief  ADC 配置（ADC1 复用初始化 + ADC2 单次转换模式）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   ADC1/CH3：与 drive_ntc.c 的 NTC 配置重复（两处均执行，等效），
 **         保留以维持本模块自洽；连续转换 + 8 次硬件平均
 **         ADC2：单次转换模式，CH4/CH5 交替使能，16 次硬件平均，
 **         转换完成触发 EOCA 中断
 ******************************************************************************/
static void Cp_ADCCfg(void)
{
	  stc_adc_init_t stcAdcInit;

    /* 1. Enable ADC1 peripheral clock. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC1, ENABLE);

    /* 2. Modify the default value depends on the application. */
    (void)ADC_StructInit(&stcAdcInit);
    stcAdcInit.u16ScanMode = ADC_MD_SEQA_CONT;
    stcAdcInit.u16Resolution = ADC_RESOLUTION_12BIT;
    stcAdcInit.u16DataAlign = ADC_DATAALIGN_RIGHT;

    /* 3. Initializes ADC. */
    (void)ADC_Init(CM_ADC1, &stcAdcInit);

    /* 4. ADC sequence configuration. */
    /* Set sample time if needed*/
    ADC_SetSampleTime(CM_ADC1, ADC_CH3, 30);

    /* ADC sequence A configuration. */
    ADC_ChCmd(CM_ADC1, ADC_SEQ_A, ADC_CH3, ENABLE);


    /* Conversion data average calculation function, if needed. */
    ADC_ConvDataAverageConfig(CM_ADC1, ADC_AVG_CNT8);
    ADC_ConvDataAverageChCmd(CM_ADC1, ADC_CH3, ENABLE);
    /* ADC Int configuration */
    ADC_IntCmd(CM_ADC1, ADC_INT_EOCA | ADC_INT_EOCB, DISABLE);
    //ADC2 config
    stc_adc_init_t stcAdc2Init;
    /* 1. Enable ADC2 peripheral clock. */
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC2, ENABLE);
    /* 2. Modify the default value depends on the application. */
    (void)ADC_StructInit(&stcAdc2Init);
    stcAdc2Init.u16ScanMode = ADC_MD_SEQA_SINGLESHOT;
    stcAdc2Init.u16Resolution = ADC_RESOLUTION_12BIT;
    stcAdc2Init.u16DataAlign = ADC_DATAALIGN_RIGHT;
    /* 3. Initializes ADC. */
    (void)ADC_Init(CM_ADC2, &stcAdc2Init);
    /* 4. ADC sequence configuration. */
    /* ADC sequence A configuration. */
    ADC_ChCmd(CM_ADC2, ADC_SEQ_A, ADC_CH4, ENABLE);
    ADC_ChCmd(CM_ADC2, ADC_SEQ_A, ADC_CH5, ENABLE);
    /* Conversion data average calculation function, if needed. */
    ADC_ConvDataAverageConfig(CM_ADC2, ADC_AVG_CNT16);
    ADC_ConvDataAverageChCmd(CM_ADC2, ADC_CH4, ENABLE);
    ADC_ConvDataAverageChCmd(CM_ADC2, ADC_CH5, ENABLE);
    /* ADC Int configuration */
    ADC_IntCmd(CM_ADC2, ADC_INT_EOCA, ENABLE);

}
/**
 *******************************************************************************
 ** \brief  CP 模块总初始化（main 初始化流程调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   先注册中断与 ADC，再启动 TMRA1 PWM 输出；启动后 1kHz 载波持续输出
 ******************************************************************************/
void drive_cpInit(void)
{
	Cp_IntCfg();
	Cp_ADCCfg();
	drive_CpInit();
}
/**
 *******************************************************************************
 ** \brief  动态调整 CP PWM 占空比（改变最大可用电流档位）
 **
 ** \param  [in] u32Value  CH5 主比较值（0~CP_TMRA_PERIOD），占空比=值/6250
 **
 ** \retval None
 ** \note   三个比较值联动：
 **         CH6 = u32Value/2        → 正半段中点采样
 **         CH7 = u32Value+(周-值)/2 → 负半段中点采样
 **         充电中如需降流/改档位调用本函数（当前业务层未使用，默认53.3%）
 ******************************************************************************/
void Cp_SetCompareValue(uint32_t u32Value)
{
	TMRA_SetCompareValue(CM_TMRA_1, TMRA_CH5,u32Value);
	 TMRA_SetCompareValue(CM_TMRA_1, TMRA_CH6,u32Value/2);
	 TMRA_SetCompareValue(CM_TMRA_1, TMRA_CH7,u32Value+(CP_TMRA_PERIOD-u32Value)/2);
}
