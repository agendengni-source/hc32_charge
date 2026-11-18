#ifndef __SER_CP_H
#define __SER_CP_H
#include "hc32_ll.h"

typedef enum
{
	CP_UNKNOWN,      // 数据无效、过期或无法识别
CP_DISCONNECTED,// 对应未连接
CP_CONNECTED,    // 对应已连接、未就绪
CP_READY,        // 对应车辆就绪
}CP_STATE;
extern CP_STATE cp_state;
void Ser_CPcheck(void);
#endif