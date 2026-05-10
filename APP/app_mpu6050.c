#include "app_global.h"

void AppTask_MPU6050Read(void *argument)
{
    float ax, ay, az, gx, gy, gz;
    uint8_t is_sleeping = 0;

    if (MPU6050_Init() == 0) { /* 初始化成功 */ }
    AppPowerGate_Acquire(APP_PGATE_MPU6050);

    for (;;)
    {
        // =======================================================
        // 1. 全局低功耗拦截：真正的 STOP 模式准备
        // =======================================================
        if (g_LowPowerMode == 1)
        {
            if (is_sleeping == 0)
            {
                AppPowerGate_Release(APP_PGATE_MPU6050);
                is_sleeping = 1;
            }
            // 只有这里才允许使用 osWaitForever！
            // 等待按键 EXTI 中断发出的信号来彻底唤醒系统
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, osWaitForever);
            continue; 
        }

        // =======================================================
        // 2. 页面拦截：系统在跑，但用户没在看我
        // =======================================================
        if (cur_page != Page_MPU6050)
        {
            if (is_sleeping == 0)
            {
                AppPowerGate_Release(APP_PGATE_MPU6050); // 不看这个页面时，让传感器闭眼省电
                is_sleeping = 1;
            }
            // 🚨 关键修复：不在页面时，每 500ms 醒来看一眼页面切回来没
            // 这样你切进页面后，最多等待 500ms 任务就会自发启动，不需要额外发信号
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 500);
            continue;
        }

        // =======================================================
        // 3. 正常运行：用户正在看 MPU6050 界面
        // =======================================================
        if (is_sleeping == 1)
        {
            AppPowerGate_Acquire(APP_PGATE_MPU6050);
            is_sleeping = 0;
        }

        // 读取并更新数据
        MPU6050_ReadSensors(&ax, &ay, &az, &gx, &gy, &gz);
        g_MPU_Data.ax = ax; g_MPU_Data.ay = ay; g_MPU_Data.az = az;
        g_MPU_Data.gx = gx; g_MPU_Data.gy = gy; g_MPU_Data.gz = gz;

        if (osMessageQueueGetCount(UI_QueueHandle) == 0)
        {
            UI_Event_t event = UI_EVENT_MPU6050_READY;
            osMessageQueuePut(UI_QueueHandle, &event, 0, 0);
        }

        // 高频采样间隔 50ms
        osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, 50); 
    }
}

