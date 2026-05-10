#include "app_global.h"

void AppTask_Key(void *argument)
{
    for (;;)
    {
        // 1. 等待外部中断唤醒
        uint32_t flags = osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);

        if ((flags & 0x80000000) == 0 && (flags & 0x0001) != 0)
        {
            if (g_LowPowerMode == 1)
            {
                // 1. 解除全局低功耗状态
                g_LowPowerMode = 0;

                // 2. 发送信号弹，唤醒所有正在挂起的兄弟任务！
                osThreadFlagsSet(Task_MPU6050Read_Handle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_DHT11Read_Handle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_RTCRead_Handle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_GuiHandle, SYS_WAKEUP_FLAG);
                osThreadFlagsSet(Task_BreathingLed_Handle, SYS_WAKEUP_FLAG);

                // 3. 吞键操作：等用户松手，防止产生多余抖动
                while (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET ||
                       HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET)
                {
                    osDelay(10);
                }

                // 清理可能产生的连带中断标志位
                osThreadFlagsWait(0x0001, osFlagsWaitAny, 0);

                // 4. 核心：直接 continue！本次按键只做“唤醒点亮屏幕”用，不触发任何菜单动作！
                continue;
            }

            osDelay(20); // 软件防抖过滤杂波

            UI_Event_t event_to_send = UI_EVENT_NONE;

            // =====================================
            // KEY0 处理：短按步进，长按连发快速滚动
            // =====================================
            if (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET)
            {
                // 1. 刚按下，立刻发送一次短按事件，让光标动一下
                event_to_send = UI_EVENT_KEY0_DOWN;
                osMessageQueuePut(UI_QueueHandle, &event_to_send, 0, 0);

                uint32_t press_start_tick = osKernelGetTickCount();

                // 只要没松手，就在这里一直循环检测
                while (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET)
                {
                    uint32_t duration = osKernelGetTickCount() - press_start_tick;

                    // 2. 如果按住超过了 500ms (长按判定阈值)
                    if (duration >= 500)
                    {
                        // 3. 进入连发模式：再发一次下移事件
                        osMessageQueuePut(UI_QueueHandle, &event_to_send, 0, 0);

                        // 4. 控制快速滚动的速度！这里设为 150ms 滚动一格。
                        // 你可以改小(比如80)滚得更快，改大(比如200)滚得更慢。
                        osDelay(150);
                    }
                    else
                    {
                        osDelay(10); // 还没到 500ms，继续等
                    }
                }

                // 松手后，把 event 置为 NONE，因为在循环里已经发过了，防止最后多发一次
                event_to_send = UI_EVENT_NONE;
            }
            // =====================================
            // 按键 KEY1 处理 (区分短按和长按)
            // =====================================
            else if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
            {
                uint32_t press_start_tick = osKernelGetTickCount();
                uint8_t is_long_press = 0;

                // 只要没松手，就在这里一直循环计时间
                while (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
                {
                    // 如果按下时间超过 800ms，判定为长按！
                    if ((osKernelGetTickCount() - press_start_tick) >= 800)
                    {
                        is_long_press = 1;
                        break; // 判定成功，立刻跳出循环
                    }
                    osDelay(10);
                }

                if (is_long_press == 1)
                {
                    event_to_send = UI_EVENT_KEY1_LONG; // 准备发送长按事件

                    // 等待用户彻底松手，否则一松手又会触发新中断
                    while (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
                    {
                        osDelay(10);
                    }
                }
                else
                {
                    event_to_send = UI_EVENT_KEY1_DOWN; // 准备发送短按事件
                }
            }

            // =====================================
            // 最终发送事件到队列
            // =====================================
            if (event_to_send != UI_EVENT_NONE)
            {
                osMessageQueuePut(UI_QueueHandle, &event_to_send, 0, 0);
            }

            // 清理彻底松手时产生的多余机械抖动标志位
            osDelay(20);
            osThreadFlagsWait(0x0001, osFlagsWaitAny, 0);
        }
    }
}
