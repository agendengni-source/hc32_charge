#include "drive_bl0939.h"
Bl0939_value blo939_value;
void drive_Bl0939Init(void)
{
	   GPIO_SetFunc(GPIO_PORT_E,GPIO_PIN_13,GPIO_FUNC_41);//SPI3-MISO
    GPIO_SetFunc(GPIO_PORT_E,GPIO_PIN_14,GPIO_FUNC_40);//SPI3-MOSI
    GPIO_SetFunc(GPIO_PORT_E,GPIO_PIN_15,GPIO_FUNC_43);//SPI3-SCK
	
	stc_spi_init_t stcSpiInit;
  stc_spi_delay_t stcSpiDelay;
	FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_SPI3, ENABLE);
    /************************* Configure SPI3***************************/
    SPI_StructInit(&stcSpiInit);
    stcSpiInit.u32WireMode = SPI_3_WIRE;
    stcSpiInit.u32TransMode = SPI_FULL_DUPLEX;
    stcSpiInit.u32MasterSlave = SPI_MASTER;
    stcSpiInit.u32Parity = SPI_PARITY_INVD;
    stcSpiInit.u32SpiMode = SPI_MD_1;
    stcSpiInit.u32BaudRatePrescaler = SPI_BR_CLK_DIV128;
    stcSpiInit.u32DataBits = SPI_DATA_SIZE_8BIT;
    stcSpiInit.u32FirstBit = SPI_FIRST_MSB;
    stcSpiInit.u32SuspendMode = SPI_COM_SUSP_FUNC_OFF;
    stcSpiInit.u32FrameLevel = SPI_1_FRAME;
    (void)SPI_Init(CM_SPI3, &stcSpiInit);
	  SPI_DelayStructInit(&stcSpiDelay);
    stcSpiDelay.u32IntervalDelay = SPI_INTERVAL_TIME_1SCK;
    stcSpiDelay.u32ReleaseDelay = SPI_RELEASE_TIME_1SCK;
    stcSpiDelay.u32SetupDelay = SPI_SETUP_TIME_1SCK;
    (void)SPI_DelayTimeConfig(CM_SPI3, &stcSpiDelay);
    /* SPI loopback function configuration */
    SPI_SetLoopbackMode(CM_SPI3, SPI_LOOPBACK_INVD);
    /* SPI parity check error self diagnosis configuration */
    SPI_ParityCheckCmd(CM_SPI3, DISABLE);
    /* Enable SPI3 */
    SPI_Cmd(CM_SPI3, ENABLE);
}
static uint8_t Rest(void)
{
	uint8_t Nuserdate[6]={0};
	uint8_t Sendbuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	return SPI_TransReceive(CM_SPI3,&Sendbuf,&Nuserdate,6,1000) ==LL_OK;
}

static uint8_t Read_Bl0939date(uint8_t addr,uint8_t *revdate)
{
	uint8_t rx[3]={0};
	uint8_t readid=ReadId;
	uint8_t Nuserdate=0;
	uint8_t Sendbuf=0xff;
	uint8_t crcdate=0;
	uint8_t cheksum;
	SPI_TransReceive(CM_SPI3,&readid,&Nuserdate,1,1000);
	SPI_TransReceive(CM_SPI3,&addr,&Nuserdate,1,1000);
	SPI_TransReceive(CM_SPI3,&Sendbuf,&rx[0],1,1000);
	SPI_TransReceive(CM_SPI3,&Sendbuf,&rx[1],1,1000);
	SPI_TransReceive(CM_SPI3,&Sendbuf,&rx[2],1,1000);
	SPI_TransReceive(CM_SPI3,&Sendbuf,&crcdate,1,1000);
	cheksum=~((readid+addr+rx[0]+rx[1]+rx[2])&0xFF);
	if(cheksum==crcdate)
	{
		revdate[0]=rx[0];
		revdate[1]=rx[1];
		revdate[2]=rx[2];
		return 1;
	}
	else
	{
		Rest();
			return 0;
	}
}

static float read_current(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	float ccdate=0;
	if(Read_Bl0939date(IA_RMS,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	ccdate=(date*1.218*2000.0)/(324004.0*1.5*1000.0);
	return ccdate;
}
	}
static float read_volate(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	float voldate=0;
	if(Read_Bl0939date(V_RMS,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	voldate=(date*1.218*400.0)/(79931*(0.0499*1000));
	return voldate;
}
	}
static float read_Power(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	float powerdate=0;
	if(Read_Bl0939date(A_WATT,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	powerdate=(date*1.218*1.218*400.0)/(4046*(0.0499*1000)*((1.5*1000)/2000));
	
	return powerdate;
}
	}
static float read_Pulsecharge(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	float Pulsecharge=0;
	if(Read_Bl0939date(CFA_CNT,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	Pulsecharge=(date*1.218*1.218*1638.4*256)/(3600000.0*4046.0*(0.0499*1000.0)*((1.5*1000)/2000));
	return Pulsecharge;
	}
}
void read_bl0903alldate(void)
{
	blo939_value.current_value=read_current();
	blo939_value.Power_value=read_Power();
	blo939_value.Pulsecharge_value=read_Pulsecharge();
	blo939_value.volate_value=read_volate();
}


