#include "app_power_gate.h"

#include "DHT11.h"
#include "i2c.h"
#include "led.h"
#include "mpu6050.h"
#include "oled.h"

static volatile uint8_t s_ref_count[APP_PGATE_COUNT];
static volatile uint8_t s_is_on[APP_PGATE_COUNT];

static uint32_t Gate_Lock(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    return primask;
}

static void Gate_Unlock(uint32_t primask)
{
    if (primask == 0U)
    {
        __enable_irq();
    }
}

static void GPIO_ToAnalog(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

static void DHT11_DataPin_Active(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
}

static void RGB_Pins_Active(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = LED_POWER_Pin | LEDR_Pin;
    HAL_GPIO_Init(LED_R_Power_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LEDB_Pin | LEDG_Pin;
    HAL_GPIO_Init(LED_B_G_GPIO_Port, &GPIO_InitStruct);
}

static void Gate_On(AppPowerGateId_t id)
{
    switch (id)
    {
    case APP_PGATE_DHT11:
        DHT11_DataPin_Active();
        DHT11_Power_Open();
        break;

    case APP_PGATE_MPU6050:
        MX_I2C2_Init();
        MPU6050_WakeUp();
        break;

    case APP_PGATE_OLED:
        MX_I2C1_Init();
        OLED_Display_On();
        break;

    case APP_PGATE_RGB_LED:
        RGB_Pins_Active();
        LED_RGB_Init();
        break;

    default:
        break;
    }
}

static void Gate_Off(AppPowerGateId_t id)
{
    switch (id)
    {
    case APP_PGATE_DHT11:
        HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET);
        GPIO_ToAnalog(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin);
        DHT11_Power_Close();
        break;

    case APP_PGATE_MPU6050:
        MPU6050_Sleep();
        HAL_I2C_DeInit(&hi2c2);
        GPIO_ToAnalog(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
        break;

    case APP_PGATE_OLED:
        OLED_Display_Off();
        HAL_I2C_DeInit(&hi2c1);
        GPIO_ToAnalog(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
        break;

    case APP_PGATE_RGB_LED:
        HAL_GPIO_WritePin(LED_R_Power_GPIO_Port, LEDR_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_B_G_GPIO_Port, LEDB_Pin | LEDG_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_R_Power_GPIO_Port, LED_POWER_Pin, GPIO_PIN_RESET);
        GPIO_ToAnalog(LED_R_Power_GPIO_Port, LEDR_Pin);
        GPIO_ToAnalog(LED_B_G_GPIO_Port, LEDB_Pin | LEDG_Pin);
        break;

    default:
        break;
    }
}

void AppPowerGate_Init(void)
{
    uint8_t i;

    for (i = 0; i < APP_PGATE_COUNT; i++)
    {
        s_ref_count[i] = 0;
        s_is_on[i] = 0;
    }

    Gate_Off(APP_PGATE_DHT11);
    Gate_Off(APP_PGATE_RGB_LED);

    s_ref_count[APP_PGATE_OLED] = 1;
    s_is_on[APP_PGATE_OLED] = 1;
}

void AppPowerGate_Acquire(AppPowerGateId_t id)
{
    uint32_t primask;
    uint8_t need_on = 0;

    if (id >= APP_PGATE_COUNT)
    {
        return;
    }

    primask = Gate_Lock();
    if (s_ref_count[id] == 0)
    {
        need_on = 1;
        s_is_on[id] = 1;
    }
    s_ref_count[id]++;
    Gate_Unlock(primask);

    if (need_on != 0)
    {
        Gate_On(id);
    }
}

void AppPowerGate_Release(AppPowerGateId_t id)
{
    uint32_t primask;
    uint8_t need_off = 0;

    if (id >= APP_PGATE_COUNT)
    {
        return;
    }

    primask = Gate_Lock();
    if (s_ref_count[id] > 0)
    {
        s_ref_count[id]--;
        if (s_ref_count[id] == 0)
        {
            need_off = 1;
            s_is_on[id] = 0;
        }
    }
    Gate_Unlock(primask);

    if (need_off != 0)
    {
        Gate_Off(id);
    }
}

uint8_t AppPowerGate_IsOn(AppPowerGateId_t id)
{
    if (id >= APP_PGATE_COUNT)
    {
        return 0;
    }

    return s_is_on[id];
}

uint8_t AppPowerGate_GetRefCount(AppPowerGateId_t id)
{
    if (id >= APP_PGATE_COUNT)
    {
        return 0;
    }

    return s_ref_count[id];
}
