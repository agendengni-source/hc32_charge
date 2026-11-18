#include "drive_cp.h"
//static void INT_SRC_TMRA_1_OVF_IrqCallback(void)
//{
//    /* USER CODE BEGIN INT_SRC_TMRA_1_OVF_IrqCallback */
//		TMRA_ClearStatus(CM_TMRA_1, TMRA_FLAG_OVF);
//    /* USER CODE END INT_SRC_TMRA_1_OVF_IrqCallback */
//}
/* 采样相位:  s_u8SamplePhase  1=CH6 触发(采正电平 CH4), 2=CH7 触发(采负电平 CH5), 0=无待读数据 */
static volatile uint8_t  s_u8SamplePhase = 0U;
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
void drive_cpInit(void)
{
	Cp_IntCfg();
	Cp_ADCCfg();
	drive_CpInit();
}
void Cp_SetCompareValue(uint32_t u32Value)
{
	TMRA_SetCompareValue(CM_TMRA_1, TMRA_CH5,u32Value);
	 TMRA_SetCompareValue(CM_TMRA_1, TMRA_CH6,u32Value/2);
	 TMRA_SetCompareValue(CM_TMRA_1, TMRA_CH7,u32Value+(CP_TMRA_PERIOD-u32Value)/2);
}
