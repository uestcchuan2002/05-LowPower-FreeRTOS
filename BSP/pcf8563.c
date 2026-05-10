#include "pcf8563.h"

/* BCD转换 */

static uint8_t DEC_To_BCD(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

static uint8_t BCD_To_DEC(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}


/**
 * @brief  初始化PCF8563
 */
HAL_StatusTypeDef PCF8563_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t data;

    /* CTRL1 = 0 */
    data = 0x00;
    if(HAL_I2C_Mem_Write(hi2c,
                         PCF8563_ADDR,
                         PCF8563_CTRL1,
                         I2C_MEMADD_SIZE_8BIT,
                         &data,
                         1,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* CTRL2 = 0 */
    data = 0x00;
    if(HAL_I2C_Mem_Write(hi2c,
                         PCF8563_ADDR,
                         PCF8563_CTRL2,
                         I2C_MEMADD_SIZE_8BIT,
                         &data,
                         1,
                         100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}


/**
 * @brief  设置时间
 */
HAL_StatusTypeDef PCF8563_SetTime(I2C_HandleTypeDef *hi2c,
                                  PCF8563_TimeTypeDef *time)
{
    uint8_t buf[7];

    buf[0] = DEC_To_BCD(time->sec);
    buf[1] = DEC_To_BCD(time->min);
    buf[2] = DEC_To_BCD(time->hour);
    buf[3] = DEC_To_BCD(time->day);
    buf[4] = DEC_To_BCD(time->week);
    buf[5] = DEC_To_BCD(time->month);
    buf[6] = DEC_To_BCD(time->year);

    return HAL_I2C_Mem_Write(hi2c,
                             PCF8563_ADDR,
                             PCF8563_SEC,
                             I2C_MEMADD_SIZE_8BIT,
                             buf,
                             7,
                             100);
}


/**
 * @brief  读取时间
 */
HAL_StatusTypeDef PCF8563_GetTime(I2C_HandleTypeDef *hi2c,
                                  PCF8563_TimeTypeDef *time)
{
    uint8_t buf[7];

    if(HAL_I2C_Mem_Read(hi2c,
                        PCF8563_ADDR,
                        PCF8563_SEC,
                        I2C_MEMADD_SIZE_8BIT,
                        buf,
                        7,
                        100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    time->sec   = BCD_To_DEC(buf[0] & 0x7F);
    time->min   = BCD_To_DEC(buf[1] & 0x7F);
    time->hour  = BCD_To_DEC(buf[2] & 0x3F);

    time->day   = BCD_To_DEC(buf[3] & 0x3F);

    time->week  = BCD_To_DEC(buf[4] & 0x07);

    time->month = BCD_To_DEC(buf[5] & 0x1F);

    time->year  = BCD_To_DEC(buf[6]);

    return HAL_OK;
}
