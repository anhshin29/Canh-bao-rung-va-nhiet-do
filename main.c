#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pcf8591.h"
#include "sensor.h"
#include "output.h"

typedef struct {
    float NhietDo;
    uint8_t DoRung;
} SensorData_t;

QueueHandle_t xDataQueue;


volatile uint8_t DEBUG_DoRung = 0; 
volatile float DEBUG_NhietDo = 0.0; 

void Sensor_Task(void *pvParameters) {
    SensorData_t myData;
    while(1) {
        myData.NhietDo = Sensor_GetTemperature();
        myData.DoRung = Sensor_GetVibration();
        
        xQueueSend(xDataQueue, &myData, 10);
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void Control_Task(void *pvParameters) {
    SensorData_t receivedData;
    uint8_t led_state = 0;
    uint8_t alarm_timer = 0;
    
    while(1) {
        if(xQueueReceive(xDataQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            
           
            DEBUG_DoRung = receivedData.DoRung;
            DEBUG_NhietDo = receivedData.NhietDo; 
            
            
            if(receivedData.DoRung > 10) {
                alarm_timer = 10; 
            }
            if(alarm_timer > 0) {
                alarm_timer--;
                led_state = !led_state; 
                Alarm_LED_Set(led_state);
            } else {
                led_state = 0;
                Alarm_LED_Set(0);
            }
            
            
            if(receivedData.NhietDo < 30.0) {
                Fan_Control(0);    
            } 
            else {
                Fan_Control(100);  
            }
        }
    }
}

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
    
    PCF8591_Init();
    Output_Init();
    
    xDataQueue = xQueueCreate(5, sizeof(SensorData_t));
    
    if(xDataQueue != NULL) {
        xTaskCreate(Sensor_Task, "Doc_Cam_Bien", 128, NULL, 1, NULL);
        xTaskCreate(Control_Task, "Dieu_Khien", 128, NULL, 2, NULL);
        
        vTaskStartScheduler();
    }
    while(1) {}
}