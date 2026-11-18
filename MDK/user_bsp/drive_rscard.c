#include "drive_rscard.h"
#include "ser_systick.h"
u3_state u3state={.u3overtime=0};
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