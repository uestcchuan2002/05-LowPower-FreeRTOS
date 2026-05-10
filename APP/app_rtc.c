#include "app_global.h"

void Read_RTC_Time(RTC_Data_t *rtc_data)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // 必须先读 Time，再读 Date，顺序绝不能变！
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // 赋值给你的全局变量
    rtc_data->year = 2000 + sDate.Year; // HAL库的年通常是 0-99
    rtc_data->month = sDate.Month;
    rtc_data->day = sDate.Date;
    rtc_data->hour = sTime.Hours;
    rtc_data->minute = sTime.Minutes;
    rtc_data->second = sTime.Seconds;
    rtc_data->weekday = sDate.WeekDay;
}

void AppTask_RTCRead(void *argument)
{
    for (;;)
    {
        // =======================================================
        // 1. 全局低功耗拦截：如果是总开关开启，立刻进入“无限期死等”
        // =======================================================
        if (g_LowPowerMode == 1)
        {
            // RTC 任务进入挂起状态，绝对不消耗 CPU
            // 直到按键任务将其唤醒并清除 g_LowPowerMode
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, osWaitForever);
            continue; // 醒来后重新检查状态
        }

        // =======================================================
        // 2. 正常工作区
        // =======================================================
        
        // 读取硬件时间
        // Read_RTC_Time(&g_RTC_Data);

        // 仅在 RTC 页面时发送刷新消息，节省队列带宽
        if (cur_page == Page_RTC)
        {
            if (osMessageQueueGetCount(UI_QueueHandle) == 0)
            {
                UI_Event_t event = UI_EVENT_RTC_READY;
                osMessageQueuePut(UI_QueueHandle, &event, 0, 0);
            }
        }

        // =======================================================
        // 3. 智能延时：实现冒号闪烁，同时保持对“低功耗切换”的秒级响应
        // =======================================================
        // 使用 500ms 超时等待。
        // 好处：如果 500ms 内没人发 Flag，它就自动醒来刷新下一秒时间；
        // 如果这 500ms 期间用户突然按下了“进入低功耗”按键，
        // 按键任务发出的 Flag 会立刻终止这个 500ms 等待，让任务直接跳回开头进入死等。
        osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 500);
    }
}
