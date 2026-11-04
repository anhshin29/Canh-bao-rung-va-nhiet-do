#ifndef __OUTPUT_H
#define __OUTPUT_H

#include "stm32f10x.h"

#define ALARM_LED_PIN   GPIO_Pin_5
#define ALARM_LED_PORT  GPIOA
#define ALARM_LED_CLK   RCC_APB2Periph_GPIOA

void Output_Init(void);
void Fan_Control(uint8_t speed);
void Alarm_LED_Set(uint8_t state);

#endif