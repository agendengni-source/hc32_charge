/**
 *******************************************************************************
 ** \brief  NTC 热敏电阻测温驱动（漏电保护器端子温度监测）
 **
 ** \file    drive_ntc.c
 ** \version V1.0
 **
 ** \note    硬件电路：3V3 — R101(5.1kΩ) — PA3 — NTC(100kΩ@25℃,B=3950) — GND
 **          NTC 接在分压下端（对地），因此温度升高 → NTC 阻值下降
 **          → PA3 电压下降 → ADC 采样值变小（与"上拉在地的接法"方向相反）
 **          测温对象为漏电保护器端子：端子松动/接触电阻增大是最典型的
 **          发热隐患，过温阈值判断在服务层 NTC_task() 中完成
 ******************************************************************************/
#include "drive_ntc.h"
#include "math.h"
#define NTC_PORT   GPIO_PORT_A
#define NTC_PIN    GPIO_PIN_03

/**
 *******************************************************************************
 ** \brief  NTC 测温 ADC 初始化（PA3 = ADC1 通道3）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PA3 配置为模拟输入；ADC1 连续转换模式 + 8 次硬件平均，
 **         采样时间 30 个 ADC 时钟（分压节点有 100nF 滤波电容，无需过长采样）
 ******************************************************************************/
void drive_NtcInit(void)
{
	stc_gpio_init_t stcGpioInit;
	 /* PA4 set to GPIO-Output */
    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;
    (void)GPIO_Init(NTC_PORT, NTC_PIN, &stcGpioInit);

	 stc_adc_init_t stcAdcInit;
    /* PA5 set to GPIO-Output */
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
//		ADC_Start(CM_ADC1);
}
//B值法
// 1/T = 1/T0 + (1/B) * 1n (R ntc/R0)  //为了将温度标度从摄氏度转换为℃到开尔文度加上数学常数273.15
 //T1是开尔文温度点
//(3.3/4096)*adc)/((3.3-(3.3/4096)*adc)/5.1)
//漏报测温
/**
 *******************************************************************************
 ** \brief  读取漏保端子温度（B 值法，100 次采样取平均）
 **
 ** \param  [in] None
 **
 ** \retval float  温度值(℃)
 ** \note   计算步骤：
 **         1) 由分压反推 NTC 阻值(kΩ)：R = 5.1 × adc / (4095 - adc)
 **         2) B 值法：1/T = 1/T0 + ln(R/R0)/B，T0=298.15K(25℃)，R0=100kΩ，B=3950
 **         3) 开尔文转摄氏：减 273.15
 **         100 次循环均值抑制 ADC 噪声；ADC 已配置 8 次硬件平均，双重滤波
 ******************************************************************************/

float NTC_NCT_RValue(void)
{
	uint32_t ch_va=0,ch_vb=0;
	float N13,N23,N33,N43;
	float BX=3950; //B25/50=3950K 25°电阻100k
	float T25=298.15;// T25 : T25=25+273.15
	float R25=100;  //25摄氏度对应NTC电阻为100k
	float Rntc=0.0;  //NTC的电阻值，单位为千欧
	float Temperature1_Sum_Value3=0;
	float rntc=0;
	static uint8_t Temperature_Counter=0;
	while(Temperature_Counter<100)
	{
		Temperature_Counter++;
		CM_ADC1->STR |=(0X1<<0);
		while(ADC_GetStatus(CM_ADC1,ADC_FLAG_EOCA)==RESET);
		ch_vb=ADC_GetValue(CM_ADC1,ADC_CH3);
		/* TODO: ch_vb 接近 4095 时（低温段 NTC 阻值暴涨）4095-ch_vb 会趋近 0，
		   导致除零/ln 溢出、温度爆值并误触发过温停机，建议增加钳位处理 */
		Rntc =5.1*ch_vb*1.0/(4095-ch_vb);
		N13=(log(Rntc)-log(R25))/BX;
		N23=1.0/T25+N13;
		N33=1.0/N23;
		N43=N33-273.15;
		Temperature1_Sum_Value3=Temperature1_Sum_Value3+N43;
	}
	Temperature_Counter=0;
	rntc=Temperature1_Sum_Value3/100.0;
	return rntc;
}
