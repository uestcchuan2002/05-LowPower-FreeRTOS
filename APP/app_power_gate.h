#ifndef __APP_POWER_GATE_H
#define __APP_POWER_GATE_H

#include "main.h"

typedef enum
{
    APP_PGATE_DHT11 = 0,
    APP_PGATE_MPU6050,
    APP_PGATE_OLED,
    APP_PGATE_RGB_LED,
    APP_PGATE_COUNT
} AppPowerGateId_t;

void AppPowerGate_Init(void);
void AppPowerGate_Acquire(AppPowerGateId_t id);
void AppPowerGate_Release(AppPowerGateId_t id);
uint8_t AppPowerGate_IsOn(AppPowerGateId_t id);
uint8_t AppPowerGate_GetRefCount(AppPowerGateId_t id);

#endif
