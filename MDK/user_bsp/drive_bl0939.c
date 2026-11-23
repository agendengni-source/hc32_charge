/**
 *******************************************************************************
 ** \brief  BL0939 电能计量芯片 SPI 驱动
 **
 ** \file    drive_bl0939.c
 ** \version V1.0
 **
 ** \note    通信规格（依据 BL0939 数据手册 V1.4 §3.1）：
 **          - SPI 从机、三线无片选、半双工，MSB 在前，固定 CPOL=0/CPHA=1（Mode1）
 **          - 读帧结构：0x55(识别) + ADDR(地址) + DATA_H/M/L(3字节数据) + CHECKSUM
 **          - 校验和 CHECKSUM = ~(0x55 + ADDR + DATA_H + DATA_M + DATA_L) & 0xFF
 **          - SPI 接口软复位：连续发送 6 字节 0xFF（校验失败后调用）
 **          换算系数依据本工程硬件（CT 2000:1 + 1.5Ω，分压 400k/49.9Ω），
 **          推导过程见各读取函数的 \note。
 ******************************************************************************/
#include "drive_bl0939.h"

/* 电参数全局缓存：供屏幕显示层与充电统计层读取 */
Bl0939_value blo939_value;

/**
 *******************************************************************************
 ** \brief  BL0939 初始化：SPI3 引脚复用与外设配置
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   引脚：PE13=SPI3_MISO，PE14=SPI3_MOSI，PE15=SPI3_SCK
 **         SPI3 时钟源 PCLK2(50MHz)，分频 DIV128 → SCLK 约 390kHz，
 **         低于 BL0939 SPI 上限 900kHz（手册§3.1），留有充分裕量
 ******************************************************************************/
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

/**
 *******************************************************************************
 ** \brief  SPI 接口软复位（连续发送 6 字节 0xFF）
 **
 ** \param  [in] None
 **
 ** \retval uint8_t  1:发送成功  0:发送失败
 ** \note   读帧校验失败后调用，用于恢复 BL0939 的 SPI 接口状态（手册§3.1.5）
 ******************************************************************************/
static uint8_t Rest(void)
{
	uint8_t Nuserdate[6]={0};
	uint8_t Sendbuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	return SPI_TransReceive(CM_SPI3,&Sendbuf,&Nuserdate,6,1000) ==LL_OK;
}

/**
 *******************************************************************************
 ** \brief  读取 BL0939 指定寄存器（数据固定 3 字节，高字节在前）
 **
 ** \param  [in]  addr     寄存器地址（见 drive_bl0939.h 中的宏定义）
 ** \param  [out] revdate  读出的 3 字节数据，revdate[0]为高字节
 **
 ** \retval uint8_t  1:校验通过  0:校验失败（已自动软复位 SPI 接口）
 ** \note   读帧时序：MCU 先发 0x55 + ADDR，随后发 4 个 0xFF 产生时钟，
 **         依次收回 DATA_H、DATA_M、DATA_L、CHECKSUM 共 4 字节
 ******************************************************************************/
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
	/* 按手册公式重算校验和并与芯片回传值比对，失败即认为本帧无效 */
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

/**
 *******************************************************************************
 ** \brief  读取 A 通道电流有效值并换算为安培
 **
 ** \param  [in] None
 **
 ** \retval float  电流值(A)；SPI 读取失败时返回上一次有效值
 ** \note   换算公式（手册§2.6）：IA_RMS = 324004 × I(mV) / Vref，Vref=1.218V
 **         本工程电流通路：CT 变比 2000:1（ZMCT156），二次侧负载 1.5Ω
 **         → 一次侧每 1A 在管脚产生 1.5Ω/2000 = 0.75mV
 **         → I(A) = date × 1.218 / (324004 × 0.75)
 **                = date × 1.218 × 2000 / (324004 × 1.5 × 1000)
 ******************************************************************************/
static float read_current(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	static float ccdate=0;                      /* 保存上次有效值，失败时沿用 */
	if(Read_Bl0939date(IA_RMS,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	ccdate=(date*1.218*2000.0)/(324004.0*1.5*1000.0);

}
	return ccdate;
	}
/**
 *******************************************************************************
 ** \brief  读取电压有效值并换算为伏特
 **
 ** \param  [in] None
 **
 ** \retval float  电压值(V)；SPI 读取失败时返回上一次有效值
 ** \note   换算公式（手册§2.6）：V_RMS = 79931 × V(mV) / Vref
 **         本工程分压：上臂 400kΩ，下臂 49.9Ω
 **         → V(V) = date × 1.218 / 79931 × (400/49.9)
 **         （分母取 400k 忽略了下臂串入的 49.9Ω，偏差约 0.01%，
 **          远小于芯片 1% 的免校准精度，可接受）
 ******************************************************************************/
static float read_volate(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	static float voldate=0;                     /* 保存上次有效值，失败时沿用 */
	if(Read_Bl0939date(V_RMS,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	voldate=(date*1.218*400.0)/(79931*(0.0499*1000));

}
	return voldate;
	}
/**
 *******************************************************************************
 ** \brief  读取 A 通道有功功率并换算为瓦特
 **
 ** \param  [in] None
 **
 ** \retval float  有功功率(W)；SPI 读取失败时返回上一次有效值
 ** \note   换算公式（手册§2.2）：WATT = 4046 × I(mV) × V(mV) / Vref²
 **         代入本工程系数（电流 0.75mV/A，电压 ×400/49.9）：
 **         P(W) = date × 1.218² × 400 / (4046 × 49.9 × 0.75)
 **         注意 A_WATT 为补码，bit23=1 表示负功，本函数未做符号处理
 ******************************************************************************/
static float read_Power(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	static float powerdate=0;                   /* 保存上次有效值，失败时沿用 */
	if(Read_Bl0939date(A_WATT,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	powerdate=(date*1.218*1.218*400.0)/(4046*(0.0499*1000)*((1.5*1000)/2000));


}
	return powerdate;
	}
/**
 *******************************************************************************
 ** \brief  读取 A 通道电能脉冲计数并换算为上电累计电量
 **
 ** \param  [in] None
 **
 ** \retval float  上电累计电量(kWh)；SPI 读取失败时返回上一次有效值
 ** \note   每脉冲电量（手册应用指南公式）：
 **         kWh/脉冲 = 1638.4×256×Vref²×(R2+R1) / (3600000×4046×RL×R1×1000)
 **         代入 RL=0.75mΩ(等效)、R1=49.9Ω、R2+R1≈400k → 4.566e-4 kWh/脉冲
 **         即每个脉冲约 0.4566 Wh，累计电量 = date × 4.566e-4
 ** \note   注意：CFA_CNT 为代数和累积（正功加、负功减，负功时向下回绕）；
 **         充电统计请使用 Meter_GetCnt() 的原始计数做差，不要用本函数的浮点值
 ******************************************************************************/
static float read_Pulsecharge(void)
{
	uint32_t date=0;
	uint8_t RevCCdate[3]={0};
	static float lastdate=0;
	if(Read_Bl0939date(CFA_CNT,RevCCdate)==1)
	{
	date=RevCCdate[0]<<16|RevCCdate[1]<<8|RevCCdate[2];
	lastdate=(date*1.218*1.218*1638.4*256*400.0)/(3600000.0*4046.0*(0.0499*1000.0)*((1.5*1000)/2000));
	}
	return lastdate;
}
/**
 *******************************************************************************
 ** \brief  读取 A 通道电能脉冲计数原始值（CFA_CNT，24bit）
 **
 ** \param  [in] None
 **
 ** \retval uint32_t  当前脉冲计数；读取失败时返回上一次有效值
 ** \note   供 ser_blo939.c 以「整数差值 × 每脉冲电量」方式统计本次充电电量：
 **         相比 float 差值方案无截断误差，且用 (int32)(now-last) 做差
 **         对 24bit 计数回绕天然安全
 ******************************************************************************/
uint32_t Meter_GetCnt(void)
{
	static uint32_t s_last = 0;
	uint8_t d[3] = {0};
	if (Read_Bl0939date(CFA_CNT, d) == 1)
	{
		s_last = ((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2];
	}
	return s_last;
}
/**
 *******************************************************************************
 ** \brief  刷新全部电参数缓存
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   函数名沿用 BL0903 例程命名习惯，实际芯片为 BL0939；
 **         由 NTC_task() 每 500ms 调用一次，
 **         SPI 读取失败时对应字段保持上次有效值（不引入垃圾数据）
 ******************************************************************************/
void read_bl0903alldate(void)
{
	blo939_value.current_value=read_current();
	blo939_value.Power_value=read_Power();
	blo939_value.Pulsecharge_value=read_Pulsecharge();
	blo939_value.volate_value=read_volate();
}

