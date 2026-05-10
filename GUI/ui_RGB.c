#include "ui_global.h"

void RGB_Contorl_Ui(void)
{
    OLED_DrawRGBMenu();
    cur_page = Page_RGBControl;
}

/**
 * @description: 绘制/刷新 RGB LED 控制界面
 */
void OLED_DrawRGBMenu(void)
{
    char disp_buf[20]; // 用于拼接字符串的缓存，OLED一行最多16个字符

    for (uint8_t i = 0; i < 4; i++)
    {
        // 选中当前行时反相高亮
        uint8_t highlight = (i == RGB_Menu_Index) ? 1 : 0;

        // 根据不同的行，判断对应的灯光状态，拼接字符串
        // 注意尾部多加几个空格，用于覆盖之前残留的字符，防止闪烁
        switch (i)
        {
            case 0:
                sprintf(disp_buf, "1. White: %s   ", led_state_white ? "ON " : "OFF");
                break;
            case 1:
                sprintf(disp_buf, "2. Red  : %s   ", led_state_red ? "ON " : "OFF");
                break;
            case 2:
                sprintf(disp_buf, "3. Blue : %s   ", led_state_blue ? "ON " : "OFF");
                break;
            case 3:
                sprintf(disp_buf, "4. Green: %s   ", led_state_green ? "ON " : "OFF");
                break;
        }

        // 显示在对应的行 (Y坐标为 0, 2, 4, 6)
        OLED_ShowString(0, i * 2, disp_buf, 16, highlight);
    }
}

/**
 * @description: 执行 RGB 菜单的互斥翻转操作 (同一时刻只能亮一个，且可单独关闭)
 */
void Execute_RGB_Action(void)
{
    // 1. 记录当前光标选中的这个灯，在按下按键前是不是亮着的？
    uint8_t was_on = 0;
    uint8_t any_on_before = led_state_white || led_state_red || led_state_blue || led_state_green;
    if (RGB_Menu_Index == 0 && led_state_white == 1) was_on = 1;
    if (RGB_Menu_Index == 1 && led_state_red == 1)   was_on = 1;
    if (RGB_Menu_Index == 2 && led_state_blue == 1)  was_on = 1;
    if (RGB_Menu_Index == 3 && led_state_green == 1) was_on = 1;

    // 2. 互斥核心：无条件把所有灯的“软件状态变量”全部清零 (全灭)
    led_state_white = 0;
    led_state_red   = 0;
    led_state_blue  = 0;
    led_state_green = 0;

    // 3. 执行硬件动作并更新状态
    if (was_on == 0)
    {
        if (any_on_before == 0)
        {
            AppPowerGate_Acquire(APP_PGATE_RGB_LED);
        }

        // 如果刚才没亮，说明用户想要【打开】它。
        // 更新对应的软件状态变量为 1，并调用对应的硬件 API。
        // (注：底层的硬件 API 会自动帮我们把其他引脚拉高熄灭，实现硬件互斥)
        switch (RGB_Menu_Index)
        {
            case 0: led_state_white = 1; LED_RGB_White(1); break;
            case 1: led_state_red   = 1; LED_RGB_Red(1);   break;
            case 2: led_state_blue  = 1; LED_RGB_Blue(1);  break;
            case 3: led_state_green = 1; LED_RGB_Green(1); break;
        }
    }
    else
    {
        // 如果刚才已经是亮的，说明用户想要【单独关闭】它。
        // 由于第 2 步软件变量已经全清 0 了，我们只需要把硬件也彻底熄灭即可。
        // 调用任何一个颜色的 0 状态（比如 White(0)），底层都会执行 LED_RGB_Set(0,0,0) 实现全灭。
        LED_RGB_White(0); 
        AppPowerGate_Release(APP_PGATE_RGB_LED);
    }

    // 4. 翻转状态后，重新刷新当前 OLED 屏幕显示 (ON/OFF)
    OLED_DrawRGBMenu();
}
