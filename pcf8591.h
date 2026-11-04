#ifndef __PCF8591_H
#define __PCF8591_H

#include "stm32f10x.h"

#define I2C_SCL_PIN      GPIO_Pin_6
#define I2C_SDA_PIN      GPIO_Pin_7
#define I2C_GPIO_PORT    GPIOB
#define I2C_GPIO_CLK     RCC_APB2Periph_GPIOB

#define PCF8591_ADDRESS  0x90 

void PCF8591_Init(void);
uint8_t PCF8591_ReadADC(uint8_t channel);
void PCF8591_WriteDAC(uint8_t val);

#endif