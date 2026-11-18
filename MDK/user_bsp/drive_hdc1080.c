#include "drive_hdc1080.h"

/* HDC1080 寄存器 */
#define HDC1080_REG_TEMP 0x00U
#define HDC1080_REG_CONFIG 0x02U
#define HDC1080_REG_DEV_ID 0xFFU /* 0x1050 */
#define I2C_TIMEOUT (2000UL)
#define RTD_ADDR (0x40U) /* 7bit 地址 */
/* ---------------- I2C 底层 (轮询, 单次 <1ms) ---------------- */
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
static int32_t hdc_write_ptr(uint8_t u8Reg)
{
	uint8_t au8Buf[1];
	au8Buf[0] = u8Reg;
	return i2c_write(au8Buf, 1U);
}

void DRV_HDC1080_Poll(HDC1080_Data_t *pstOut)
{
	static uint8_t s_u8Phase = 0U;
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
/* 阶段2: 读取温湿度 4 字节 (距触发已 500ms >> 14ms 转换时间) */
if (i2c_read(au8Data, 4U) == LL_OK) 
	{
	u16RawT = ((uint16_t)au8Data[0] << 8) | au8Data[1];
	u16RawH = ((uint16_t)au8Data[2] << 8) | au8Data[3];
	s_stData.fTemp = ((float)u16RawT / 65536.0f) * 165.0f - 40.0f;
	s_stData.fHumi = ((float)u16RawH / 65536.0f) * 100.0f;
	s_stData.u8Valid = 1U;
} 
	s_u8Phase = 0U;
} 
	*pstOut = s_stData;
}



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
