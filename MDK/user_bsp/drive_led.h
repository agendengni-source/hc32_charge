#ifndef __DRIVE_LED_H
#define __DRIVE_LED_H
#include "hc32_ll.h"
typedef enum
{
	LED_RED=0,
	LED_GREEN,
	LED_BLUE,
}led_id;
typedef enum
{
	led_off =0,
	led_on =1,
}LED_StateTypedef;
typedef struct
{
	uint8_t LED_Port;
	uint16_t LED_Pin;
	LED_StateTypedef state;
}LED_objtypedef;


#define LED_BLUE_PORT   GPIO_PORT_B
#define LED_BLUE_PIN    GPIO_PIN_04

#define LED_RED_PORT   GPIO_PORT_B
#define LED_RED_PIN    GPIO_PIN_03

#define LED_GREEN_PORT   GPIO_PORT_D
#define LED_GREEN_PIN    GPIO_PIN_07

void drive_LedInit(void);
void LedOn(led_id id);
void LedOff(led_id id);
void ledTuen(led_id id);
#endif