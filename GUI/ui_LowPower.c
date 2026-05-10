#include "ui_global.h"

uint8_t g_LowPowerMode = 0;

/**
 * @description: 绘制/刷新低功耗设置界面
 */
void OLED_DrawLowPowerPage(void)
{
    // 静态文字（如果在进入界面时已经清过屏了，这里直接写就行）
    OLED_ShowString(18, 2, "--Low Power--", 16, 0);
    
    // 动态状态（ON 后面带一个空格，是为了盖住 OFF 的第三个字母，防止字符残留）
    if (g_LowPowerMode == 0)
    {
       
        OLED_ShowString(64, 4, "OFF ", 16, 0); 
    }
    else
    {   
        
        OLED_ShowString(64, 4, "ON ", 16, 0);
		
    }
	
	cur_page = lowPowerPage;
}

/**
 * @description: 执行低功耗开关的翻转操作
 */
void Execute_LowPower_Toggle(void)
{
    // 1. 切换全局状态
    g_LowPowerMode = !g_LowPowerMode;

	if (g_LowPowerMode == 1)
	{
        uint8_t rgb_was_on = led_state_white || led_state_red || led_state_blue || led_state_green;

		led_state_white = 0;
		led_state_red   = 0;
		led_state_blue  = 0;
		led_state_green = 0;
		LED_RGB_Set(0, 0, 0);	
        if (rgb_was_on != 0)
        {
            AppPowerGate_Release(APP_PGATE_RGB_LED);
        }

        
	}
	
}
