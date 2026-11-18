#include "drive_ntc.h"
#include "math.h"
#define NTC_PORT   GPIO_PORT_A
#define NTC_PIN    GPIO_PIN_03

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