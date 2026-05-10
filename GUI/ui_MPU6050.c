#include "ui_global.h"

MPU6050_Data_t g_MPU_Data;    // 定义全局变量
uint8_t mpu_display_mode = 0; // 0:显示加速度计, 1:显示陀螺仪

/**
 * @description: 绘制/刷新 MPU6050 数据监控界面
 */
void OLED_DrawMPU6050Page(void)
{
    char disp_buf[20];
    if (g_LowPowerMode == 0)
    {
        if (mpu_display_mode == 0)
        {
            // --- 页面 1：加速度计 ---
            OLED_ShowString(0, 0, "--- Accel ---   ", 16, 1); // 顶部高亮标题

            // %6.2f：总宽度6，保留2位小数。尾部多加几个空格覆盖旧字符
            sprintf(disp_buf, "X:%6.2f g     ", g_MPU_Data.ax);
            OLED_ShowString(0, 2, disp_buf, 16, 0);

            sprintf(disp_buf, "Y:%6.2f g     ", g_MPU_Data.ay);
            OLED_ShowString(0, 4, disp_buf, 16, 0);

            sprintf(disp_buf, "Z:%6.2f g     ", g_MPU_Data.az);
            OLED_ShowString(0, 6, disp_buf, 16, 0);
        }
        else
        {
            // --- 页面 2：陀螺仪 ---
            OLED_ShowString(0, 0, "--- Gyro ---    ", 16, 1);

            // 陀螺仪数值较大，用 %7.1f
            sprintf(disp_buf, "X:%7.1f dps ", g_MPU_Data.gx);
            OLED_ShowString(0, 2, disp_buf, 16, 0);

            sprintf(disp_buf, "Y:%7.1f dps ", g_MPU_Data.gy);
            OLED_ShowString(0, 4, disp_buf, 16, 0);

            sprintf(disp_buf, "Z:%7.1f dps ", g_MPU_Data.gz);
            OLED_ShowString(0, 6, disp_buf, 16, 0);
        }
    }
    else
    {
        OLED_ShowString(25, 3, "Low Power...", 16, 0);
    }
}
