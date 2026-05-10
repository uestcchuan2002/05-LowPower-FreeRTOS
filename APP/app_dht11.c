#include "app_global.h"

SenserDate g_SensorData;

#include "app_global.h"

void AppTask_DHT11Read(void *argument)
{
    uint8_t is_sleeping = 1;
    AppPowerGate_Release(APP_PGATE_DHT11);

    for (;;)
    {
        // =======================================================
        // 情况 1：全局低功耗模式 -> 彻底死等，准备进入 STOP
        // =======================================================
        if (g_LowPowerMode == 1)
        {
            if (is_sleeping == 0)
            {
                AppPowerGate_Release(APP_PGATE_DHT11);
                is_sleeping = 1;
            }
            // 只有总开关按下时，才无限期阻塞，等待按键中断发 Flag 叫醒
            uint32_t flags = osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 30000);

            if (flags == osFlagsErrorTimeout) 
            {
                g_LowPowerMode = 0;
                is_sleeping = 0;
                osThreadFlagsSet(Task_MPU6050Read_Handle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_RTCRead_Handle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_GuiHandle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_BreathingLed_Handle, SYS_WAKEUP_FLAG);
            }
            continue; 
        }

        // =======================================================
        // 情况 2：正常运行模式，但用户在看别的页面 -> 轮询待机
        // =======================================================
        if (cur_page != Page_DHT11_Data)
        {
            if (is_sleeping == 0)
            {
                AppPowerGate_Release(APP_PGATE_DHT11); // 不在页面时断电省电
                is_sleeping = 1;
            }
            // 🚨 修复关键：不在页面时不能死等！每 500ms 醒来看看页面切过来没
            // 用带超时的 Wait，如果期间按了低功耗开关，也能秒响应
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 500);
            continue;
        }

        // =======================================================
        // 情况 3：用户就在温湿度页面 -> 全力运行
        // =======================================================
        if (is_sleeping == 1)
        {
            AppPowerGate_Acquire(APP_PGATE_DHT11);
            // 上电需要等 1.5s 稳定。使用 Wait 替代 Delay，保持对总开关的秒响应
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 1500); 
            is_sleeping = 0;
        }

        DHT11_REC_Data();

        if (dht11_error_code == 0)
        {
            g_SensorData.temp = rec_data[2];
            g_SensorData.humi = rec_data[0];

            if (osMessageQueueGetCount(UI_QueueHandle) == 0)
            {
                UI_Event_t event = UI_EVENT_DHT11_READY;
                osMessageQueuePut(UI_QueueHandle, &event, 0, 0);
            }
        }

        // 正常采样间隔 1.5s
        osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 1500); 
    }
}
