/**
 *******************************************************************************
 ** \brief  急停按钮消抖服务接口
 **
 ** \file    ser_estop.h
 ** \version V1.0
 **
 ** \note    三态消抖状态机：释放 -> 按下确认(20ms) -> 等待释放；
 **          由 ESTOP_task 每 1ms 推进一次，确认按下后持续返回 1
 ******************************************************************************/
#ifndef __SER_ESTOP_H
#define __SER_ESTOP_H
#include "hc32_ll.h"

#define CONFIRM_TIME	20       // 按下消抖阈值，单位 ms
/* 消抖阶段依次为已释放、等待按下确认、已确认并等待释放。 */
typedef enum
{
    ESTOP_RELEASED,
    ESTOP_PRESSED,
    ESTOP_WAIT_RELEASE,
} ESTOP_STATE;
/* 保存消抖阶段和首次检测到按下的时间戳。 */
typedef struct
{
    ESTOP_STATE state;
    uint32_t release_tick;   // 首次检测到按下的时刻，单位 ms
} ESTOP_FILTER;

/* 推进急停消抖并读取结果；确认后持续按下时返回 1。 */
extern uint8_t Key_state(void);
#endif
