#ifndef __SENSOR_H
#define __SENSOR_H

#include "stm32f10x.h"
#include "pcf8591.h"

float Sensor_GetTemperature(void);
uint8_t Sensor_GetVibration(void);

#endif