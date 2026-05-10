#ifndef __APP_GLOBAL_H   
#define __APP_GLOBAL_H   

#include "oled.h"
#include <stdio.h>
#include "ui_global.h"
#include "DHT11.h"
#include "led.h"
#include "mpu6050.h"
#include "rtc.h"
#include "app_power_gate.h"

typedef struct 
{
    unsigned int temp;
    unsigned int humi;
} SenserDate;

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
} MPU6050_Data_t;


typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  weekday; // 1-7 (1代表周一)
} RTC_Data_t;

#define SENSOR_WAKEUP_FLAG 0x0002
#define SYS_WAKEUP_FLAG 0x0001

extern SenserDate g_SensorData;

extern osThreadId_t Task_DHT11Read_Handle;
extern osThreadId_t Task_MPU6050Read_Handle;
extern osThreadId_t Task_RTCRead_Handle;
extern osThreadId_t Task_GuiHandle;
extern osThreadId_t Task_BreathingLed_Handle;

extern MPU6050_Data_t g_MPU_Data; 
extern uint8_t mpu_display_mode;
extern RTC_Data_t g_RTC_Data;

void AppTask_Key(void *argument);
void AppTask_BreathingLed(void *argument);
void AppTask_DHT11Read(void *argument);
void AppTask_RTCRead(void *argument);
void AppTask_MPU6050Read(void *argument);

#endif 
