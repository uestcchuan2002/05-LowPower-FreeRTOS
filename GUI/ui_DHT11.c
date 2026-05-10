#include "ui_global.h"

void DHT11_Data_Ui(uint8_t g_LowPowerMode)
{
    if (g_LowPowerMode == 0)
    {
        OLED_ShowString(16, 2, "Temp: ", 16, 0);
        OLED_ShowNum(64, 2, 0, 2, 16, 0);
        OLED_ShowChar(85, 2, 'C', 16, 0); // 模拟显示 25C

        OLED_ShowString(16, 4, "Humi: ", 16, 0);
        OLED_ShowNum(64, 4, 0, 2, 16, 0);
        OLED_ShowChar(85, 4, '%', 16, 0); // 模拟显示 60%
    }
    else
    {
        OLED_ShowString(25, 3, "Low Power...", 16, 0);
    }

    cur_page = Page_DHT11_Data;
}
