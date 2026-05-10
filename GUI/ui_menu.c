#include "ui_global.h"

char *Menu_Items[MENU_ITEM_COUNT] = {
    "1. DHT11 Data  ",
    "2. RGB Control  ",
    "3. MPU6050 Data ",
    "4. RTC Time     ",
    "5. WiFi Config  ",
    "6. Low Power  ",
    "7. Factory Reset"};

/**
 * @description: 绘制/刷新滚动菜单界面
 */
void OLED_DrawMenu(void)
{
    for (uint8_t i = 0; i < PAGE_SIZE; i++)
    {
        // 计算当前物理行(i)对应的实际菜单项索引
        uint8_t item_idx = Show_Offset + i;

        if (item_idx < MENU_ITEM_COUNT)
        {
            // 如果实际索引在菜单总数内，正常显示
            uint8_t highlight = (item_idx == Menu_Index) ? 1 : 0;
            OLED_ShowString(0, i * 2, Menu_Items[item_idx], 16, highlight);
        }
        else
        {
            // 如果超出了菜单总数（比如一共 6 项，滑到最后时底部会有空行）
            // 用 16 个空格覆盖，达到清除该行的效果
            OLED_ShowString(0, i * 2, "                ", 16, 0);
        }
    }
}

/**
 * @description: 执行对应菜单项的功能
 * @param {uint8_t} index 当前菜单项的索引
 */
void Execute_Menu_Action(uint8_t index)
{
    OLED_Clear(); // 进入具体功能前清屏

    switch (index)
    {
    case 0: // 1. Sensor Data (传感器数据)
        DHT11_Data_Ui(g_LowPowerMode);
    break;

    case 1: // 2. LED Control (LED 控制)
        RGB_Contorl_Ui();
        break;

    case 2: 
        OLED_DrawMPU6050Page();
		cur_page = Page_MPU6050;
        break;

    case 3: 
        OLED_DrawRTCPage();
		cur_page = Page_RTC;
        break;

    case 4: 
        OLED_ShowString(16, 3, "AP: My_WiFi", 16, 0);
        cur_page = Page5;
        break;

    case 5: 
    {
        OLED_DrawLowPowerPage();
    }

    break;

    case 6: // 7. Factory Reset (恢复出厂设置)
        OLED_ShowString(16, 3, "Reset Done!", 16, 0);
        cur_page = Page7;
        break;

    default:
        break;
    }
    // 执行完功能（用户按下按键确认退出）后，清屏并重新画出主菜单
}
