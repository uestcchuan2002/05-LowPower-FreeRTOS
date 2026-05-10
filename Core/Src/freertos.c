/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
#include <oled.h>
#include <stdio.h>
#include "DHT11.h"
#include "app_global.h"
#include "ui_global.h"


// 呼吸灯任务参数
osThreadId_t Task_BreathingLed_Handle;
const osThreadAttr_t Task_BreathingLed_attributes = {
    .name = "Task_BreathingLed",
    .stack_size = 128 * 1,
    .priority = (osPriority_t)osPriorityLow,
};
// 按键任务参数
osThreadId_t Task_KeyHandle;
const osThreadAttr_t Task_Key_attributes = {
    .name = "Task_Key",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityBelowNormal,    // 优先级16
};
// GUI显示任务参数
osThreadId_t Task_GuiHandle;
const osThreadAttr_t Task_Gui_attributes = {
    .name = "Task_Gui",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityLow1,    // 优先级9
};
// DHT11读取任务
osThreadId_t Task_DHT11Read_Handle;
const osThreadAttr_t Task_DHT11Read_attributes = {
    .name = "Task_DHT11Read",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow6,   // 优先级14
};
// MPU6050读取任务
osThreadId_t Task_MPU6050Read_Handle;
const osThreadAttr_t Task_MPU6050Read_attributes = {
    .name = "Task_MPU6050Read",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityLow6,   // 优先级14
};
// RTC读取任务
osThreadId_t Task_RTCRead_Handle;
const osThreadAttr_t Task_RTCRead_attributes = {
    .name = "Task_RTCRead",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow6,   // 优先级14
};


osMessageQueueId_t UI_QueueHandle = NULL;

void AppTask_BreathingLed(void *argument);
void AppTask_Key(void *argument);
void AppTask_Gui(void *argument);
void AppTask_DHT11Read(void *argument);
void AppTask_MPU6050Read(void *argument);
void AppTask_RTCRead(void *argument);

void MX_FREERTOS_Init(void); 

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    UI_QueueHandle = osMessageQueueNew(16, sizeof(UI_Event_t), NULL);
    Task_BreathingLed_Handle = osThreadNew(AppTask_BreathingLed, NULL, &Task_BreathingLed_attributes);
    Task_KeyHandle = osThreadNew(AppTask_Key, NULL, &Task_Key_attributes);
    Task_GuiHandle = osThreadNew(AppTask_Gui, NULL, &Task_Gui_attributes);
    Task_DHT11Read_Handle = osThreadNew(AppTask_DHT11Read, NULL, &Task_DHT11Read_attributes);
    Task_MPU6050Read_Handle = osThreadNew(AppTask_MPU6050Read, NULL, &Task_MPU6050Read_attributes);
    Task_RTCRead_Handle = osThreadNew(AppTask_RTCRead, NULL, &Task_RTCRead_attributes);
}

/**
 * @brief  空闲任务钩子函数
 * @param  None
 * @retval None
 */
void  vApplicationIdleHook(void )
{
	static uint32_t led_counter = 0;
	led_counter++;
	
	if (led_counter >= 1000000) 
	{
		LED0_Troggle();
		led_counter = 0;
	}
}

/*
    低功耗程序处理
*/
#if (configUSE_TICKLESS_IDLE != 0)
/* 
    1.专门用于从 STOP 模式唤醒后的时钟恢复 (无延时死锁，不破坏 SysTick)
    2.如果使用SystemClock_Config()来进行系统时钟恢复的话，很可能出现死锁
    3.应该为上述函数依赖与滴答定时器的计时，但此时刚刚恢复，滴答定时器也还没有配置好，因此会出现问题
    4.下面的函数时直接通过Hal库操作寄存器来进行的，主要完成了HSE和PLL的开启，和将主时钟源切换为PLL
    5.因为之前的参数仍留在寄存器中，因此不需要重写设置分频参数
*/
static void Restore_F103_Clock_After_WakeUp(void)
{
    /* 1. 开启 HSE (外部高速时钟) */
    __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
    
    /* 2. 死等 HSE 准备就绪 (纯硬件标志位轮询，不依赖滴答定时器) */
    while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET) 
    {
        // 阻塞等待，实际物理耗时极短，通常在微秒级别
    }
    
    /* 3. 开启主 PLL */
    __HAL_RCC_PLL_ENABLE();
    
    /* 4. 等待主 PLL 准备就绪 */
    while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET) 
    {
		
    }
    
    /* 5. 将系统主时钟源切换回 PLL */
    __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);
    
    /* 6. 确认时钟源切换成功 */
    while(__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK) 
    {
		
    }
}

/*
    1.对官方的函数弱函数进行重写
    2.主要从之前的由滴答定时器进行计时，改为了使用外部RTC进行进行计时
*/
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    uint32_t ulStartRTC, ulEndRTC, ulRTC_Diff;
    TickType_t xActualSleepTicks;

    // 1. 关中断，防止在休眠前夕发生任务调度
    __disable_irq();

    // 2. 再次确认是否真的需要休眠
    if (eTaskConfirmSleepModeStatus() == eAbortSleep)
    {
        __enable_irq();
        return;
    }

    // 读取休眠前的 RTC 计数值
    ulStartRTC = (RTC->CNTH << 16) | RTC->CNTL;

    // ==========================================
    // 💤 睡前准备：防溢出判断 & 设置闹钟
    // ==========================================
    // 🌟 终极防弹补丁：如果预期是死等（无限期休眠）
    // 或者预期时间大到会导致乘法溢出（比如大于4000000ms，即4000秒）
    if (xExpectedIdleTime == portMAX_DELAY || xExpectedIdleTime > 4000000)
    {
        // 彻底关闭闹钟，不设闹钟了，纯靠按键 EXTI 唤醒！
        __HAL_RTC_ALARM_DISABLE_IT(&hrtc, RTC_IT_ALRA);
    }
    else
    {
        // 正常定时休眠：换算并设置闹钟 (不会溢出了)
        uint32_t counts_to_sleep = (xExpectedIdleTime * 1024) / 1000;
        uint32_t alarm_target = ulStartRTC + counts_to_sleep;

        // 配置闹钟底层寄存器
        while((RTC->CRL & RTC_CRL_RTOFF) == 0);
        RTC->CRL |= RTC_CRL_CNF;         
        RTC->ALRH = alarm_target >> 16;    
        RTC->ALRL = alarm_target & 0xFFFF; 
        RTC->CRL &= ~RTC_CRL_CNF;        
        while((RTC->CRL & RTC_CRL_RTOFF) == 0);

        // 开启闹钟中断与 EXTI17 唤醒线
        __HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRA);
        EXTI->IMR |= EXTI_IMR_MR17;
        EXTI->RTSR |= EXTI_RTSR_TR17;
    }

    // ==========================================
    // 🛌 熄灯睡觉：进入 STOP 模式
    // ==========================================
    // 停掉操作系统的滴答定时器
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    // 进入深度睡眠
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    // ==========================================
    // ☀️ 醒来后的第一件事：时钟与心跳复苏
    // ==========================================
    // 第一时间把单片机从内部 8MHz 拉回 72MHz！
    Restore_F103_Clock_After_WakeUp();

    // 立刻恢复操作系统的滴答定时器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // ==========================================
    // 🚨 灭杀“幽灵闹钟”
    // ==========================================
    __HAL_RTC_ALARM_DISABLE_IT(&hrtc, RTC_IT_ALRA);
    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF); 
    EXTI->PR = EXTI_PR_PR17;                           

    // ==========================================
    // ⏱️ 算账与时间补偿
    // ==========================================
    // 同步等待 RTC 寄存器更新完毕
    RTC->CRL &= (uint16_t)~RTC_CRL_RSF;
    while((RTC->CRL & RTC_CRL_RSF) == 0);

    // 读取醒来后的 RTC 计数值
    ulEndRTC = (RTC->CNTH << 16) | RTC->CNTL;

    // 计算走过的计数值
    ulRTC_Diff = ulEndRTC - ulStartRTC;

    // 换算回 RTOS 的 OS Tick (1000Hz)
    xActualSleepTicks = (ulRTC_Diff * 1000) / 1024;

    // 安全防线：实际补偿的时间绝不能超过预期时间
    if( xActualSleepTicks > xExpectedIdleTime )
    {
        xActualSleepTicks = xExpectedIdleTime; 
    }

    // 一口气把失去的时间补偿给内核
    vTaskStepTick( xActualSleepTicks );

    // 3. 开中断，系统全面复活
    __enable_irq();
}

#endif



