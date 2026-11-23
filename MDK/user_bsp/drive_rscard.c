/**
 *******************************************************************************
 ** \brief  射频读卡模块串口驱动（USART3 + 帧超时判定）
 **
 ** \file    drive_rscard.c
 ** \version V1.0
 **
 ** \note    帧接收机制（无 DMA，中断逐字节）：
 **          - 收到首字节：置 u3flag=1，SysTick 开始计帧内间隔
 **          - 每收 1 字节：u3overtime 清零（9600bps 下字节间隔约 1ms）
 **          - 5ms 无新字节：SysTick 置 u3overtimeflag=1，视为一帧接收完成
 **          帧解析与白名单校验在服务层 s_rscard.c 的 compdate() 中完成
 ******************************************************************************/
#include "drive_rscard.h"
#include "ser_systick.h"
u3_state u3state={.u3overtime=0};
/**
 *******************************************************************************
 ** \brief  USART3 接收中断：逐字节写入接收缓冲区
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   datelen 到 1 时启动帧超时计数；缓冲区写满（128字节）整帧作废清零，
 **         防止异常数据撑爆缓冲区；清 RX_FULL 标志后退出
 ******************************************************************************/
static void INT_SRC_USART3_RI_IrqCallback(void)
{
  u3state.u3rx_datebuf[u3state.datelen++]= USART_ReadData(CM_USART3);
	if(u3state.datelen==1)
	{
		u3state.u3flag=1;
	}
	else if(u3state.datelen>=datelenmax)
	{
		u3state.datelen=0;
	}
	USART_ClearStatus(CM_USART3,USART_FLAG_RX_FULL);
	u3state.u3overtime=0;

}
/**
 *******************************************************************************
 ** \brief  USART3 接收中断注册
 **
 ** \param  [in] None
 **
 ** \retval None
 ******************************************************************************/
void drive_u3IntCfg(void)
{
	stc_irq_signin_config_t stcIrq;
	 stcIrq.enIntSrc = INT_SRC_USART3_RI;
    stcIrq.enIRQn = INT086_IRQn;
    stcIrq.pfnCallback = &INT_SRC_USART3_RI_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrq);
    /* NVIC config */
    NVIC_ClearPendingIRQ(INT086_IRQn);
    NVIC_SetPriority(INT086_IRQn, DDL_IRQ_PRIO_15);
    NVIC_EnableIRQ(INT086_IRQn);


}
/**
 *******************************************************************************
 ** \brief  USART3 外设初始化（读卡模块 9600-8-N-1）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PB6=USART3_TX(GPIO_FUNC_32)，PB7=USART3_RX(GPIO_FUNC_33)；
 **         时钟分频 DIV16 + 16 倍过采样得到 9600bps
 ******************************************************************************/
void drive_RscardInit(void)
{
	GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_06,GPIO_FUNC_32);//USART3-TX
  GPIO_SetFunc(GPIO_PORT_B,GPIO_PIN_07,GPIO_FUNC_33);//USART3-RX
	stc_usart_uart_init_t stcUartInit;
	FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART3, ENABLE);
    /************************* Configure USART3***************************/
    USART_DeInit(CM_USART3);
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockSrc = USART_CLK_SRC_INTERNCLK;
    stcUartInit.u32ClockDiv = USART_CLK_DIV16;
    stcUartInit.u32CKOutput = USART_CK_OUTPUT_DISABLE;
    stcUartInit.u32Baudrate = 9600UL;
    stcUartInit.u32DataWidth = USART_DATA_WIDTH_8BIT;
    stcUartInit.u32StopBit = USART_STOPBIT_1BIT;
    stcUartInit.u32Parity = USART_PARITY_NONE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_16BIT;
    stcUartInit.u32FirstBit = USART_FIRST_BIT_LSB;
    stcUartInit.u32StartBitPolarity = USART_START_BIT_FALLING;
    stcUartInit.u32HWFlowControl = USART_HW_FLOWCTRL_RTS;
    USART_UART_Init(CM_USART3, &stcUartInit, NULL);
    /* Enable USART_TX | USART_RX | USART_INT_RX function */
    USART_FuncCmd(CM_USART3, (USART_TX | USART_RX | USART_INT_RX), ENABLE);
}
