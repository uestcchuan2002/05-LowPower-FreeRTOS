/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    rtc.c
 * @brief   This file provides code for the configuration
 *          of the RTC instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

    /* USER CODE BEGIN RTC_Init 0 */

    /* USER CODE END RTC_Init 0 */

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef DateToUpdate = {0};

    /* USER CODE BEGIN RTC_Init 1 */

    /* USER CODE END RTC_Init 1 */

    /** Initialize RTC Only
     */
    hrtc.Instance = RTC;
    /* 将RTC的频率设置为1KHz */
    hrtc.Init.AsynchPrediv = 31;

    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;

    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }

    /* USER CODE BEGIN Check_RTC_BKUP */

    /* USER CODE END Check_RTC_BKUP */

    /** Initialize RTC and set the Time and Date
     */
    sTime.Hours = 0x14;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
    }
    DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
    DateToUpdate.Month = RTC_MONTH_MAY;
    DateToUpdate.Date = 0x4;
    DateToUpdate.Year = 0x26;

    if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN RTC_Init 2 */
    // 确保在操作备份域前开启相关时钟和访问权限
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_BKP_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // 读取备份寄存器1，判断是否已经被初始化过
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x5050)
    {
        if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x5050)
        {
            // 【首次上电】或【后备电池没电了】进行初始化设置

            RTC_TimeTypeDef sTime = {0};
            RTC_DateTypeDef sDate = {0};

            // 1. 设置时间：14时30分00秒
            sTime.Hours = 14;
            sTime.Minutes = 30;
            sTime.Seconds = 0;
            // 注意：F103 的 RTC 只有秒计数器，HAL 库通过软件处理时间格式
            if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
            {
                Error_Handler();
            }

            // 2. 设置日期：2026年5月4日 星期一
            sDate.Year = 26; // 代表 2026 年
            sDate.Month = RTC_MONTH_MAY;
            sDate.Date = 4;
            sDate.WeekDay = RTC_WEEKDAY_MONDAY;
            if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
            {
                Error_Handler();
            }

            // 3. 写入备份寄存器标记，防止下次复位后时间被重置
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5050);
        }
    }
    else
    {
    }
    /* USER CODE END RTC_Init 2 */
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *rtcHandle)
{

    if (rtcHandle->Instance == RTC)
    {
        /* USER CODE BEGIN RTC_MspInit 0 */

        /* USER CODE END RTC_MspInit 0 */
        HAL_PWR_EnableBkUpAccess();
        /* Enable BKP CLK enable for backup registers */
        __HAL_RCC_BKP_CLK_ENABLE();
        /* RTC clock enable */
        __HAL_RCC_RTC_ENABLE();

        
        HAL_NVIC_SetPriority(RTC_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(RTC_IRQn);
        
        HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
    }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef *rtcHandle)
{

    if (rtcHandle->Instance == RTC)
    {
        /* USER CODE BEGIN RTC_MspDeInit 0 */

        /* USER CODE END RTC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();

        /* RTC interrupt Deinit */
        HAL_NVIC_DisableIRQ(RTC_IRQn);
        /* USER CODE BEGIN RTC_MspDeInit 1 */

        /* USER CODE END RTC_MspDeInit 1 */
    }
}

// 传入你想睡的毫秒数 (比如 500ms)
void Set_Next_RTC_Alarm_ms(uint32_t ms_to_sleep)
{
    // 1. 读取当前的 32 位 RTC 计数值 (此时单位已经是 1/1024 秒了)
    uint32_t current_time = (RTC->CNTH << 16) | RTC->CNTL;
    
    // 2. 毫秒换算：将 1000Hz 的毫秒数，换算成 1024Hz 的计数值
    // 公式： RTC计数 = (毫秒数 * 1024) / 1000
    uint32_t count_to_add = (ms_to_sleep * 1024) / 1000;
    
    // 3. 计算出应该醒来的目标计数值
    uint32_t alarm_time = current_time + count_to_add;
    
    // ... 后续的寄存器写入逻辑保持不变 ...
    while((RTC->CRL & RTC_CRL_RTOFF) == 0) {}
    RTC->CRL |= RTC_CRL_CNF;         
    RTC->ALRH = alarm_time >> 16;    
    RTC->ALRL = alarm_time & 0xFFFF; 
    RTC->CRL &= ~RTC_CRL_CNF;        
    while((RTC->CRL & RTC_CRL_RTOFF) == 0) {}
    
    __HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRA);
    EXTI->IMR |= EXTI_IMR_MR17;
    EXTI->RTSR |= EXTI_RTSR_TR17;
}


