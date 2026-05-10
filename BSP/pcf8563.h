#ifndef __PCF8563_H
#define __PCF8563_H

#include "stm32f1xx_hal.h"

/* PCF8563 I2C地址 */
#define PCF8563_ADDR      (0x51)

/* 寄存器地址 */
#define PCF8563_CTRL1     0x00
#define PCF8563_CTRL2     0x01

#define PCF8563_SEC       0x02
#define PCF8563_MIN       0x03
#define PCF8563_HOUR      0x04
#define PCF8563_DAY       0x05
#define PCF8563_WEEK      0x06
#define PCF8563_MONTH     0x07
#define PCF8563_YEAR      0x08

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;

    uint8_t week;

    uint8_t hour;
    uint8_t min;
    uint8_t sec;

}PCF8563_TimeTypeDef;


/* API */

HAL_StatusTypeDef PCF8563_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef PCF8563_SetTime(I2C_HandleTypeDef *hi2c,
                                  PCF8563_TimeTypeDef *time);

HAL_StatusTypeDef PCF8563_GetTime(I2C_HandleTypeDef *hi2c,
                                  PCF8563_TimeTypeDef *time);

#endif
