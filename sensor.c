#include "sensor.h"

float Sensor_GetTemperature(void) {
    uint8_t adc_val = PCF8591_ReadADC(0); 
    return (adc_val * 500.0) / 255.0;
}

uint8_t Sensor_GetVibration(void) {
    uint8_t min_val = 255;
    uint8_t max_val = 0;
    uint8_t current_val = 0;

    for(int i = 0; i < 30; i++) {
        current_val = PCF8591_ReadADC(1); 
        if(current_val > max_val) max_val = current_val;
        if(current_val < min_val) min_val = current_val;
    }
    return (max_val - min_val); 
}