#ifndef __DRIVE_RELAY_H
#define __DRIVE_RELAY_H
#include "hc32_ll.h"
  #define Relay_L_PORT   GPIO_PORT_A
#define Relay_L_PIN    GPIO_PIN_04

#define Relay_N_PORT   GPIO_PORT_A
#define Relay_N_PIN    GPIO_PIN_05
#define NRealyON() GPIO_SetPins(Relay_N_PORT,Relay_N_PIN)
#define NRealyOFF() GPIO_ResetPins(Relay_N_PORT,Relay_N_PIN)

#define LRealyON() GPIO_SetPins(Relay_L_PORT,Relay_L_PIN)
#define LRealyOFF() GPIO_ResetPins(Relay_L_PORT,Relay_L_PIN)
void drive_RelayInit(void);
en_pin_state_t realaystate_L(void);
en_pin_state_t realaystate_N(void);
#endif
