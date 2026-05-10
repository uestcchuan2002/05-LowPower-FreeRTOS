#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f1xx_hal.h"
#include "main.h"
#include "cmsis_os.h"

extern unsigned int rec_data[4];
extern uint8_t dht11_error_code;

#define DHT11_DATA_Pin        GPIO_PIN_12
#define DHT11_DATA_GPIO_Port  GPIOB

// 宏必须加括号！！！
#define dht11_high()    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET)
#define dht11_low()     HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET)
#define Read_Data()     HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin)

void DHT11_Power_Open(void);
void DHT11_Power_Close(void);
void DHT11_REC_Data(void);

#endif
