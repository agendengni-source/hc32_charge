/**
 *******************************************************************************
 ** \brief  系统时钟配置：XTAL 8MHz → PLL → 200MHz
 **
 ** \file    drive_clkcfg.c
 ** \version V1.0
 **
 ** \note    PLL 倍频路径：8MHz / M(1) × N(50) / P(2) = 200MHz
 **          总线分频（官方 200MHz 例程标准配置）：
 **          HCLK=200M，EXCLK=100M，PCLK0=200M，PCLK1=100M，
 **          PCLK2=50M，PCLK3=50M，PCLK4=100M
 **          —— USART1/3、SPI3/4 挂 PCLK 分支，波特率分频按此推算
 ******************************************************************************/
#include "drive_clkcfg.h"



/**
 *******************************************************************************
 ** \brief  时钟系统初始化（main 最先调用）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   顺序：总线分频 → SRAM/Flash 等待周期（200MHz 下必须配置，
 **         否则取指出错）→ 开 XTAL 稳定 2ms → 配置并启动 PLL
 **         → GPIO 读等待 → 电源高速模式 → 切换系统时钟源到 PLL
 ******************************************************************************/
void drive_ClkcfgInit(void)
{
	     CLK_SetClockDiv(CLK_BUS_CLK_ALL, (CLK_HCLK_DIV1 | CLK_EXCLK_DIV2 | CLK_PCLK0_DIV1 | CLK_PCLK1_DIV2 | \
                                   CLK_PCLK2_DIV4 | CLK_PCLK3_DIV4 | CLK_PCLK4_DIV2));
    /* sram init include read/write wait cycle setting */
    SRAM_SetWaitCycle(SRAM_SRAM_ALL, SRAM_WAIT_CYCLE1, SRAM_WAIT_CYCLE1);
    SRAM_SetWaitCycle(SRAM_SRAMH, SRAM_WAIT_CYCLE0, SRAM_WAIT_CYCLE0);
    /* flash read wait cycle setting */
    EFM_SetWaitCycle(EFM_WAIT_CYCLE5);
    /* XTAL config */
    stc_clock_xtal_init_t stcXtalInit;
    (void)CLK_XtalStructInit(&stcXtalInit);
    stcXtalInit.u8State = CLK_XTAL_ON;
    stcXtalInit.u8Drv = CLK_XTAL_DRV_HIGH;
    stcXtalInit.u8Mode = CLK_XTAL_MD_OSC;
    stcXtalInit.u8StableTime = CLK_XTAL_STB_2MS;
    (void)CLK_XtalInit(&stcXtalInit);
    /* MPLL config */
    stc_clock_pll_init_t stcMPLLInit;
    (void)CLK_PLLStructInit(&stcMPLLInit);
    stcMPLLInit.PLLCFGR = 0UL;
    stcMPLLInit.PLLCFGR_f.PLLM = (1UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLN = (50UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLP = (2UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLQ = (2UL - 1UL);
    stcMPLLInit.PLLCFGR_f.PLLR = (2UL - 1UL);
    stcMPLLInit.u8PLLState = CLK_PLL_ON;
    stcMPLLInit.PLLCFGR_f.PLLSRC = CLK_PLL_SRC_XTAL;
    (void)CLK_PLLInit(&stcMPLLInit);
    /* 3 cycles for 126MHz ~ 200MHz */
    GPIO_SetReadWaitCycle(GPIO_RD_WAIT3);
    /* Switch driver ability */
    PWC_HighSpeedToHighPerformance();
    /* Set the system clock source */
    CLK_SetSysClockSrc(CLK_SYSCLK_SRC_PLL);
}
