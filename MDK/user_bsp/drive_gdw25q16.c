#include "drive_gdw25q16.h"

static void CSHIG(void)
{
	GPIO_SetPins(GD25Q16CS_PORT,GD25Q16CS_PIN);
}
static void CSLOW(void)
{
	GPIO_ResetPins(GD25Q16CS_PORT,GD25Q16CS_PIN);
}

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