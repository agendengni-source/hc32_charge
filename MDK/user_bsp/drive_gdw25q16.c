/**
 *******************************************************************************
 ** \brief  GD25Q16 SPI NOR Flash 驱动
 **
 ** \file    drive_gdw25q16.c
 ** \version V1.0
 **
 ** \note    SPI4 配置：Mode0（CPOL=0/CPHA=0，GD25Qxx 标准）、MSB 在前、
 **          分频 DIV16（PCLK2 50MHz/16 ≈ 3.1MHz，Flash 可承受）；
 **          片选 PD8 由 GPIO 手动控制（外设 3 线模式无硬件 CS）
 **          当前只实现了 JEDEC ID 读取用于"芯片在位"自检，
 **          页读写/擦除待充电记录功能开发时补充
 ******************************************************************************/
#include "drive_gdw25q16.h"

/* 片选拉高（总线释放，Flash 不选中） */
static void CSHIG(void)
{
	GPIO_SetPins(GD25Q16CS_PORT,GD25Q16CS_PIN);
}
/* 片选拉低（选中 Flash，开始命令传输） */
static void CSLOW(void)
{
	GPIO_ResetPins(GD25Q16CS_PORT,GD25Q16CS_PIN);
}

/**
 *******************************************************************************
 ** \brief  GD25Q16/SPI4 初始化
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   CS 引脚初始化为输出且默认高电平（PIN_STAT_SET，不选中），
 **          再配置 SPI4 主模式全双工
 ******************************************************************************/
void drive_Gd25q16Init(void)
{

    GPIO_SetFunc(GPIO_PORT_D,GPIO_PIN_09,GPIO_FUNC_45);//SPI4-MISO
    GPIO_SetFunc(GPIO_PORT_D,GPIO_PIN_10,GPIO_FUNC_44);//SPI4-MOSI
    GPIO_SetFunc(GPIO_PORT_D,GPIO_PIN_11,GPIO_FUNC_47);//SPI4-SCK
	 /* GPIO initialize */
    stc_gpio_init_t stcGpioInit;
    /* PH2 set to GPIO-Output */
  (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinState = PIN_STAT_SET;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    (void)GPIO_Init(GD25Q16CS_PORT, GD25Q16CS_PIN, &stcGpioInit);
	 stc_spi_init_t stcSpiInit;
    stc_spi_delay_t stcSpiDelay;
	 FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_SPI4, ENABLE);
    /************************* Configure SPI1***************************/
    SPI_StructInit(&stcSpiInit);
    stcSpiInit.u32WireMode = SPI_3_WIRE;
    stcSpiInit.u32TransMode = SPI_FULL_DUPLEX;
    stcSpiInit.u32MasterSlave = SPI_MASTER;
    stcSpiInit.u32Parity = SPI_PARITY_INVD;
    stcSpiInit.u32SpiMode = SPI_MD_0;
    stcSpiInit.u32BaudRatePrescaler = SPI_BR_CLK_DIV16;
    stcSpiInit.u32DataBits = SPI_DATA_SIZE_8BIT;
    stcSpiInit.u32FirstBit = SPI_FIRST_MSB;
    stcSpiInit.u32SuspendMode = SPI_COM_SUSP_FUNC_OFF;
    stcSpiInit.u32FrameLevel = SPI_1_FRAME;
    (void)SPI_Init(CM_SPI4, &stcSpiInit);

    SPI_DelayStructInit(&stcSpiDelay);
    stcSpiDelay.u32IntervalDelay = SPI_INTERVAL_TIME_1SCK;
    stcSpiDelay.u32ReleaseDelay = SPI_RELEASE_TIME_1SCK;
    stcSpiDelay.u32SetupDelay = SPI_SETUP_TIME_1SCK;
    (void)SPI_DelayTimeConfig(CM_SPI4, &stcSpiDelay);

    /* SPI loopback function configuration */
    SPI_SetLoopbackMode(CM_SPI4, SPI_LOOPBACK_INVD);
    /* SPI parity check error self diagnosis configuration */
    SPI_ParityCheckCmd(CM_SPI4, DISABLE);
    /* Enable SPI1 */
    SPI_Cmd(CM_SPI4, ENABLE);
}
/**
 *******************************************************************************
 ** \brief  读取 GD25Q16 器件 ID（JEDEC ID，命令 0x9F）
 **
 ** \param  [in] None
 **
 ** \retval uint32_t  24bit ID：[23:16]=厂家(GD 为 0xC8)，[15:8]=容量类型，[7:0]=容量
 ** \note   时序：CS拉低 → 发 0x9F → 连续发 3 个 0xFF 产生时钟收 ID → CS拉高；
 **         RTC_task 周期调用本函数兼作 Flash 在位自检（ID 变化说明异常）
 ******************************************************************************/
uint32_t GD25Q_ReadJEDEC_ID(void)
{
    uint8_t cmd = 0x9F;
    uint8_t rx[3] = {0};
    uint8_t dummy = 0xFF;
    CSLOW();
    // 发送命令 0x9F
    SPI_TransReceive(CM_SPI4, &cmd, &dummy, 1, 1000);
    // 连续读 3 个字节，发送 0xFF 产生时钟
    SPI_TransReceive(CM_SPI4, &dummy, &rx[0], 1, 1000);
    SPI_TransReceive(CM_SPI4, &dummy, &rx[1], 1, 1000);
    SPI_TransReceive(CM_SPI4, &dummy, &rx[2], 1, 1000);
    CSHIG();
    // 组合成 32 位 ID
    return ((uint32_t)rx[0] << 16) | ((uint32_t)rx[1] << 8) | rx[2];
}
