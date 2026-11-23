/**
 *******************************************************************************
 ** \brief  HDC1080 温湿度传感器驱动（I2C3 硬件外设）
 **
 ** \file    drive_hdc1080.c
 ** \version V1.0
 **
 ** \note    器件规格（依据 TI HDC1080 数据手册）：
 **          - 7bit I2C 地址 0x40，指针寄存器架构（写寄存器号→读写数据）
 **          - 14bit 温度转换 6.35ms + 14bit 湿度转换 6.50ms，合计约 13ms
 **          - 上电后需等待约 15ms 才能开始首次测量
 **          轮询机制：DRV_HDC1080_Poll() 为两阶段状态机，先触发转换，
 **          下次调用（200ms 后）再读结果——等待时间远大于转换时间，
 **          由服务层 HDC1080_task() 以 200ms 周期调用
 ******************************************************************************/
#include "drive_hdc1080.h"

/* HDC1080 寄存器 */
#define HDC1080_REG_TEMP 0x00U           /* 温度寄存器（写指针到0x00即触发转换） */
#define HDC1080_REG_CONFIG 0x02U         /* 配置寄存器（分辨率/测量模式/加热器） */
#define HDC1080_REG_DEV_ID 0xFFU /* 0x1050 */ /* 器件ID寄存器，用于在线校验芯片在位 */
#define I2C_TIMEOUT (2000UL)             /* I2C 硬件操作超时（硬件计数单位） */
#define RTD_ADDR (0x40U) /* 7bit 地址 */ /* HDC1080 7bit 器件地址 */
/* ---------------- I2C 底层 (轮询, 单次 <1ms) ---------------- */
/**
 *******************************************************************************
 ** \brief  I2C 写数据底层（自动启停，带软件复位防总线卡死）
 **
 ** \param  [in] pu8Buf  待写数据缓冲区
 ** \param  [in] u32Len  数据长度（字节）
 **
 ** \retval int32_t  LL_OK:成功  其它:失败码
 ** \note   每次传输前做一次 I2C 软件复位，防止从机拉死总线后外设卡住
 ******************************************************************************/
static int32_t i2c_write(const uint8_t *pu8Buf, uint32_t u32Len)
{
int32_t i32Ret;
I2C_Cmd(CM_I2C3, ENABLE);
I2C_SWResetCmd(CM_I2C3, ENABLE);
I2C_SWResetCmd(CM_I2C3, DISABLE);
i32Ret = I2C_Start(CM_I2C3, I2C_TIMEOUT);
if (i32Ret == LL_OK) {
i32Ret = I2C_TransAddr(CM_I2C3, RTD_ADDR, I2C_DIR_TX, I2C_TIMEOUT);
if (i32Ret == LL_OK)
	{
i32Ret = I2C_TransData(CM_I2C3, (uint8_t *)pu8Buf, u32Len,
I2C_TIMEOUT);
}
} (
void)I2C_Stop(CM_I2C3, I2C_TIMEOUT);
I2C_Cmd(CM_I2C3, DISABLE);
return i32Ret;
}
/**
 *******************************************************************************
 ** \brief  I2C 读数据底层（自动启停，失败时补发 STOP 释放总线）
 **
 ** \param  [out] pu8Buf  读回数据缓冲区
 ** \param  [in]  u32Len  读取长度（字节）
 **
 ** \retval int32_t  LL_OK:成功  其它:失败码
 ******************************************************************************/
static int32_t i2c_read(uint8_t *pu8Buf, uint32_t u32Len)
{
int32_t i32Ret;
I2C_Cmd(CM_I2C3, ENABLE);
I2C_SWResetCmd(CM_I2C3, ENABLE);
I2C_SWResetCmd(CM_I2C3, DISABLE);
i32Ret = I2C_Start(CM_I2C3, I2C_TIMEOUT);
if (i32Ret == LL_OK) {
i32Ret = I2C_TransAddr(CM_I2C3, RTD_ADDR, I2C_DIR_RX, I2C_TIMEOUT);
if (i32Ret == LL_OK) {
i32Ret = I2C_MasterReceiveDataAndStop(CM_I2C3, pu8Buf, u32Len,
I2C_TIMEOUT);
}
}
if (i32Ret != LL_OK) {
(void)I2C_Stop(CM_I2C3, I2C_TIMEOUT);
}
	I2C_Cmd(CM_I2C3, DISABLE);
	return i32Ret;
}
/**
 *******************************************************************************
 ** \brief  写指针寄存器（指向目标寄存器；指向0x00时同时触发一次转换）
 **
 ** \param  [in] u8Reg  目标寄存器地址
 **
 ** \retval int32_t  LL_OK:成功  其它:失败码
 ******************************************************************************/
static int32_t hdc_write_ptr(uint8_t u8Reg)
{
uint8_t au8Buf[1];
au8Buf[0] = u8Reg;
return i2c_write(au8Buf, 1U);
}

/**
 *******************************************************************************
 ** \brief  HDC1080 两阶段轮询读取（非阻塞，每次调用只做半个事务）
 **
 ** \param  [out] pstOut  本次轮询时的最新温湿度快照（可能仍是上次结果）
 **
 ** \retval None
 ** \note   阶段0：写指针0x00触发温度+湿度转换（配置为温+湿同测模式）
 **         阶段1：读回4字节（温度MSB/LSB+湿度MSB/LSB）并换算物理量
 **         两次调用间隔由任务表保证（200ms >> 13ms转换时间）；
 **         I2C失败时 s_stData 保持上次有效值，u8Valid 标记数据是否曾就绪
 ******************************************************************************/


void DRV_HDC1080_Poll(HDC1080_Data_t *pstOut)
{
static uint8_t s_u8Phase = 0U;
static uint8_t s_u8ErrCnt = 0U;   /* 连续I2C事务失败计数 */
static HDC1080_Data_t s_stData;
uint8_t au8Data[4];
uint16_t u16RawT, u16RawH;
if (pstOut == NULL)
	{
return;
}
if (s_u8Phase == 0U)
	{
/* 阶段1: 指向温度寄存器, 触发转换 (转换在后台进?) */
if (hdc_write_ptr(HDC1080_REG_TEMP) == LL_OK)
	{
s_u8Phase = 1U;
}
}
else {
/* 阶段2: 读取温湿度 4 字节 (距触发已≥200ms任务周期 >> 13ms转换时间) */
if (i2c_read(au8Data, 4U) == LL_OK)
	{
		
	s_u8ErrCnt = 0U;                       /* 成功清零失败计数 */
u16RawT = ((uint16_t)au8Data[0] << 8) | au8Data[1];
u16RawH = ((uint16_t)au8Data[2] << 8) | au8Data[3];
/* 手册换算式：T(℃)=raw/65536×165-40；RH(%RH)=raw/65536×100 */
s_stData.fTemp = ((float)u16RawT / 65536.0f) * 165.0f - 40.0f;
s_stData.fHumi = ((float)u16RawH / 65536.0f) * 100.0f;
s_stData.u8Valid = 1U;
}
s_u8Phase = 0U;
s_u8ErrCnt++;                          /* 失败累加，供业务层判传感器故障 */
}
s_stData.u8Err = s_u8ErrCnt;
*pstOut = s_stData;
}



/**
 *******************************************************************************
 ** \brief  I2C3 外设初始化（400kHz 快速模式）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   PE2=I2C3_SDA(GPIO_FUNC_48)，PE3=I2C3_SCL(GPIO_FUNC_49)；
 **         使能总线等待（BusWait），时钟分频1、SclTime=0 按手册默认时序
 ******************************************************************************/
void drive_Hdc1080Init(void)
{
	  GPIO_SetFunc(GPIO_PORT_E,GPIO_PIN_02,GPIO_FUNC_48);//I2C3-SDA
    GPIO_SetFunc(GPIO_PORT_E,GPIO_PIN_03,GPIO_FUNC_49);//I2C3-SCL
	 int32_t i32Ret;
    stc_i2c_init_t stcI2cInit;
    float32_t fErr;

    /* Enable I2C3 clock */
    FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_I2C3, ENABLE);
    /************************* Configure I2C3***************************/
    (void)I2C_DeInit(CM_I2C3);
    (void)I2C_StructInit(&stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV1;
    stcI2cInit.u32Baudrate = 400000UL;
    stcI2cInit.u32SclTime = 0UL;
    i32Ret = I2C_Init(CM_I2C3, &stcI2cInit, &fErr);
    if (LL_OK != i32Ret) {
        //Initialized failed, add your code here.
    }
    I2C_BusWaitCmd(CM_I2C3, ENABLE);
}
/**
 *******************************************************************************
 ** \brief  HDC1080 上电配置：器件ID校验 + 分辨率设置（main 初始化时调用一次）
 **
 ** \param  [in] None
 **
 ** \retval None
 ** \note   流程：初始化I2C3 → 指向DEV_ID(0xFF) → 读2字节校验0x1050
 **         → 写配置寄存器：MODE=00(温+湿同测)、TRES/HRES=14bit、加热器关
 **         ID 校验失败不致命（温湿度保护将失效），仅跳过配置
 ******************************************************************************/
void DRV_HDC1080_Config()
{
drive_Hdc1080Init();
uint8_t au8Buf[3];
uint8_t au8Id[2];
/* 器件 ID 校验 (0x1050), 失败不致命, 仅标记 */
if (hdc_write_ptr(HDC1080_REG_DEV_ID) == LL_OK) {
for (volatile uint32_t d = 0; d < 20000U; d++) { __NOP(); }
if ((i2c_read(au8Id, 2U) == LL_OK) && (au8Id[0] == 0x10U) && (au8Id[1]
== 0x50U)) {
/* 配置: 14bit 温度+湿度, 加热器关 */
au8Buf[0] = HDC1080_REG_CONFIG;
au8Buf[1] = 0x10U; /* MODE=00(温+湿), TRES/HRES=14bit */
au8Buf[2] = 0x00U;
(void)i2c_write(au8Buf, 3U);
}
}
}
