#include "ui_global.h"

RTC_Data_t g_RTC_Data;

// 星期转换表 (注意对应你的 weekday 数字逻辑，这里假设 1=Mon, 7=Sun)
const char* Weekday_Str[8] = {"ERR", "Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat.", "Sun."};

/**
 * @description: 绘制/刷新 RTC 实时时钟界面
 */
void OLED_DrawRTCPage(void)
{
    char disp_buf[20];

    // 1. 第一行：标题栏 (反相高亮)
    
    OLED_ShowString(0, 0, "---Local Time---", 16, 1);
    
    // 2. 第二行：日期 (YYYY-MM-DD 居中显示)
    sprintf(disp_buf, "   %04d-%02d-%02d   ", g_RTC_Data.year, g_RTC_Data.month, g_RTC_Data.day);
    OLED_ShowString(0, 2, disp_buf, 16, 0);

    // 3. 第三行：时间 (HH:MM:SS 居中显示)
    // 酷炫特效：让冒号根据秒数的奇偶来闪烁
    if (g_RTC_Data.second % 2 == 0) {
        sprintf(disp_buf, "    %02d:%02d:%02d    ", g_RTC_Data.hour, g_RTC_Data.minute, g_RTC_Data.second);
    } else {
        sprintf(disp_buf, "    %02d %02d %02d    ", g_RTC_Data.hour, g_RTC_Data.minute, g_RTC_Data.second);
    }
    OLED_ShowString(0, 4, disp_buf, 16, 0);

    // 4. 第四行：星期 (居中显示)
    // 防止数组越界保护
    uint8_t wd = (g_RTC_Data.weekday >= 1 && g_RTC_Data.weekday <= 7) ? g_RTC_Data.weekday : 0;
    sprintf(disp_buf, "    [ %s ]    ", Weekday_Str[wd]);
    OLED_ShowString(0, 6, disp_buf, 16, 0);
}

