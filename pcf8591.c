#include "pcf8591.h"

void I2C_Delay(void) {
    volatile int i = 100; // ау tang delay d? ?n d?nh nhi?u
    while(i--);
}

void PCF8591_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(I2C_GPIO_CLK, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN | I2C_SDA_PIN);
}

void I2C_Start(void) {
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
}

void I2C_Stop(void) {
    GPIO_ResetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    I2C_Delay();
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
    I2C_Delay();
}

uint8_t I2C_WriteByte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if (byte & 0x80) GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
        else GPIO_ResetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
        byte <<= 1;
        I2C_Delay();
        GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
        I2C_Delay();
        GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    }
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    I2C_Delay();
    uint8_t ack = !GPIO_ReadInputDataBit(I2C_GPIO_PORT, I2C_SDA_PIN);
    GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    return ack;
}

uint8_t I2C_ReadByte(uint8_t ack) {
    uint8_t byte = 0;
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN);
    for (int i = 0; i < 8; i++) {
        byte <<= 1;
        GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
        I2C_Delay();
        if (GPIO_ReadInputDataBit(I2C_GPIO_PORT, I2C_SDA_PIN)) byte |= 0x01;
        GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
        I2C_Delay();
    }
    
    if(ack) {
        GPIO_ResetBits(I2C_GPIO_PORT, I2C_SDA_PIN); 
    } else {
        GPIO_SetBits(I2C_GPIO_PORT, I2C_SDA_PIN); 
    }
    
    GPIO_SetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    I2C_Delay();
    GPIO_ResetBits(I2C_GPIO_PORT, I2C_SCL_PIN);
    return byte;
}

void PCF8591_WriteDAC(uint8_t val) {
    I2C_Start();
    I2C_WriteByte(PCF8591_ADDRESS);
    I2C_WriteByte(0x40); 
    I2C_WriteByte(val);
    I2C_Stop();
}

uint8_t PCF8591_ReadADC(uint8_t channel) {
    uint8_t val;
    I2C_Start();
    I2C_WriteByte(PCF8591_ADDRESS);
    I2C_WriteByte(0x40 | (channel & 0x03)); 
    I2C_Start();
    I2C_WriteByte(PCF8591_ADDRESS | 0x01);
    
    I2C_ReadByte(1); 
    val = I2C_ReadByte(0); 
    
    I2C_Stop();
    return val;
}