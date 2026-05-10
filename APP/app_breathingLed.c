#include "app_global.h"

void AppTask_BreathingLed(void *argument)
{
    for (;;)
    {
        // =======================================================
        // 模式 A：正常运行，LED 闪烁
        // =======================================================
        if (g_LowPowerMode == 0) 
        {
            LED0_Troggle(); // 翻转 LED
            
            // 🌟 绝招：用带超时的 Wait 代替 osDelay(2000)
            // 好处是：在等待的这 2 秒内，如果用户按下了“低功耗总开关”，
            // 这个任务能被立刻中断唤醒，瞬间切到关灯状态，零延迟！
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 2000);
        }
        // =======================================================
        // 模式 B：全局低功耗 -> 关灯并“死等”
        // =======================================================
        else 
        {
            LED0(0); // 关灯 (注意确认 0 是灭还是亮，通常高电平 1 是灭)
            
            // 🚨 核心改造：进入无限期挂起！绝不消耗 CPU！
            // 直到按键任务给它发送 SYS_WAKEUP_FLAG 才会重新活过来。
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, osWaitForever);
        }
    }
}


