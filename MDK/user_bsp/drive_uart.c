/**
 *******************************************************************************
 ** \brief  调试/串口屏 UART 驱动（USART1，printf 重定向）
 **
 ** \file    drive_uart.c
 ** \version V1.0
 **
 ** \note    用途：printf 输出淘晶驰串口屏指令（格式：控件名="内容"\xff\xff\xff）
 **          引脚：PD1=USART1_RX(GPIO_FUNC_32)，PD2=USART1_TX(GPIO_FUNC_33)
 **          波特率 115200-8-N-1，内部时钟源 16 倍过采样
 **          注意：发送为阻塞式（fputc 忙等 TX_CPLT），屏幕刷新频繁时
 **          会占用主循环时间，指令应尽量精简
 ******************************************************************************/
#include "drive_uart.h"
#include "stdio.h"
 static void INT_SRC_USART1_RI_IrqCallback(void)
{
    /* USER CODE BEGIN INT_SRC_USART1_RI_IrqCallback */

    /* USER CODE END INT_SRC_USART1_RI_IrqCallback */
}
/* INT_SRC_USART1_EI Callback. */
static void INT_SRC_USART1_EI_IrqCallback(void)
{
    /* USER CODE BEGIN INT_SRC_USART1_EI_IrqCallback */

    /* USER CODE END INT_SRC_USART1_EI_IrqCallback */
}

/**
 *******************************************************************************
 ** \brief  USART1 初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   配置 115200-8-N-1；同时使能 TX/RX/RX中断 功能位
 **         （虽然开了 RX 中断，但 drive_IntCfg 当前未在 main 中调用，
 **          收方向未真正启用——业务上本串口只发不收）
 ******************************************************************************/
void drive_UartInit(void)
{
	  GPIO_SetFunc(GPIO_PORT_D,GPIO_PIN_01,GPIO_FUNC_32);//USART1-RX
    GPIO_SetFunc(GPIO_PORT_D,GPIO_PIN_02,GPIO_FUNC_33);//USART1-TX
	   stc_usart_uart_init_t stcUartInit;

    /* Enable USART1 clock */
    FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART1, ENABLE);
    /************************* Configure USART1***************************/
    USART_DeInit(CM_USART1);
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockSrc = USART_CLK_SRC_INTERNCLK;
    stcUartInit.u32ClockDiv = USART_CLK_DIV1;
    stcUartInit.u32CKOutput = USART_CK_OUTPUT_DISABLE;
    stcUartInit.u32Baudrate = 115200UL;
    stcUartInit.u32DataWidth = USART_DATA_WIDTH_8BIT;
    stcUartInit.u32StopBit = USART_STOPBIT_1BIT;
    stcUartInit.u32Parity = USART_PARITY_NONE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_16BIT;
    stcUartInit.u32FirstBit = USART_FIRST_BIT_LSB;
    stcUartInit.u32StartBitPolarity = USART_START_BIT_FALLING;
    stcUartInit.u32HWFlowControl = USART_HW_FLOWCTRL_RTS;
    USART_UART_Init(CM_USART1, &stcUartInit, NULL);
    /* Enable USART_TX | USART_RX | USART_INT_RX function */
    USART_FuncCmd(CM_USART1, (USART_TX | USART_RX | USART_INT_RX), ENABLE);
}
/**
 *******************************************************************************
 ** \brief  USART1 中断注册（RI/EI 空回调，预留扩展）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   当前 main() 中 drive_IntCfg() 处于注释状态，未实际启用
 ******************************************************************************/
void drive_IntCfg(void)
{
    stc_irq_signin_config_t stcIrq;

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_RI;
    stcIrq.enIRQn = INT080_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_RI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT080_IRQn);
    NVIC_SetPriority(INT080_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT080_IRQn);

    /* IRQ sign-in */
    stcIrq.enIntSrc = INT_SRC_USART1_EI;
    stcIrq.enIRQn = INT081_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART1_EI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT081_IRQn);
    NVIC_SetPriority(INT081_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT081_IRQn);

}


/**
 *******************************************************************************
 ** \brief  printf 字符输出重定向（Keil 微库 Retarget）
 **
 ** \param  [in] c       待发送字符
 ** \param  [in] stream  文件流（忽略）
 **
 ** \retval int  原样返回字符
 ** \note   忙等 USART1 发送完成标志后写入数据寄存器，逐字符阻塞发送
 ******************************************************************************/
int fputc(int c,FILE *stream)
{
	while(USART_GetStatus(CM_USART1,USART_INT_TX_CPLT)==RESET)
	{

	}
	USART_WriteData(CM_USART1, (uint16_t)c);
	return c;
}
