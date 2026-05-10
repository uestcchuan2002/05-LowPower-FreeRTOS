#include "main.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

#include <stdio.h>
#include <oled.h>
#include <led.h>
#include "DHT11.h"
#include "mpu6050.h"
#include "pcf8563.h"
#include "app_power_gate.h"

#pragma import(__use_no_semihosting)

void _sys_exit(int x)
{
    x = x;
}

struct __FILE
{
    int handle;
};

FILE __stdout;

int fputc(int ch, FILE *stream)
{

    while ((USART1->SR & 0X40) == 0)
        ;

    USART1->DR = (uint8_t)ch;
    return ch;
}

// 微秒延时函数（RTOS 安全）
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t start;

    // 时钟频率 = 系统时钟 (例如 72MHz / 168MHz)
    ticks = nus * (SystemCoreClock / 1000000);
    start = SysTick->VAL;

    while ((SysTick->VAL - start) & 0x00FFFFFF < ticks)
        ;
}

int main(void)
{

    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    LED_RGB_Init();

    MX_USART1_UART_Init();
    MX_RTC_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();

    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    OLED_Init();
    OLED_Clear();

    LED0_Init();
    AppPowerGate_Init();

    // 如果需要在低功耗期间进行程序下载，需要开启下面函数
    // HAL_DBGMCU_EnableDBGSleepMode();

    /*
        电源led电流计算：
        1000Ω
        1.49V
        1.49ma
    */

    osKernelInitialize();
    MX_FREERTOS_Init();
    osKernelStart();

    while (1)
    { // 必须先读 Time，再读 Date，否则底层影子寄存器会锁死导致时间不更新
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        printf("20%02d-%02d-%02d %02d:%02d:%02d\n", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
        HAL_Delay(2000);
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
