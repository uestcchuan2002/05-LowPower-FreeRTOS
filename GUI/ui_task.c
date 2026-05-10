#include "ui_global.h"

uint8_t Menu_Index = 0;  // 当前选中的光标索引 (0 到 MENU_ITEM_COUNT-1)
uint8_t Show_Offset = 0; // 屏幕最顶端显示的菜单项索引 (窗口顶端位置)

// 记录四个灯的状态，0为关闭，1为开启
uint8_t led_state_white = 0;
uint8_t led_state_red   = 0;
uint8_t led_state_blue  = 0;
uint8_t led_state_green = 0;
// 这个页面的专用光标索引 (独立于主菜单的光标)
uint8_t RGB_Menu_Index = 0;

currentPage cur_page = menuPage;

void AppTask_Gui(void *argument)
{
    OLED_Init();
    OLED_Clear();
    OLED_DrawMenu();

    // 记录 OLED屏幕状态
    uint8_t oled_is_sleep = 0;
    uint8_t oled_gate_on = 1;

    for (;;)
    {
        /*
            判断是否开启了低功耗模式，
            如果开启了则显示任务进入阻塞状态
            如果没有开启，则进入下面的正常菜单显示
        */
        if (g_LowPowerMode == 1)
        {
            if (oled_is_sleep == 0)
            {
                oled_is_sleep = 1;
                if (oled_gate_on != 0)
                {
                    AppPowerGate_Release(APP_PGATE_OLED);
                    oled_gate_on = 0;
                }
            }

            /* GUI 任务进入无限期死等，绝不吃 CPU！*/
            osThreadFlagsWait(SYS_WAKEUP_FLAG, osFlagsWaitAny, osWaitForever);
            /* 醒来后重新判断状态，而非阻塞 */
            continue; // 
        }
        else
        {
            if (oled_is_sleep == 1)
            {
                /* 更新oled任务的状态：醒来 */
                oled_is_sleep = 0;
                if (oled_gate_on == 0)
                {
                    AppPowerGate_Acquire(APP_PGATE_OLED);
                    oled_gate_on = 1;
                }
                /* 开启OLED屏幕 */
                OLED_Display_On();
                /* 醒来直接绘制菜单页面 */
				Menu_Index = 0;
				Show_Offset = 0;
                cur_page = menuPage;
                OLED_DrawMenu();
                
            }
        }

        // =======================================================
        // 2. 正常消息接收与处理
        // =======================================================
        UI_Event_t recv_event = UI_EVENT_NONE;
        // 核心阻塞点：死等消息队列，直到有按键或传感器发来通知
        osMessageQueueGet(UI_QueueHandle, &recv_event, NULL, osWaitForever);

        if (recv_event == UI_EVENT_KEY1_LONG)
        {
            if (cur_page != menuPage) // 如果本来就在主菜单就不重复刷了
            {
                cur_page = menuPage;
                OLED_Clear();
                OLED_DrawMenu();
            }
            continue; // 关键：跳过本次循环后面的所有代码，重新去等新消息！
        }

        if (cur_page == menuPage)
        {
            if (recv_event == UI_EVENT_KEY0_DOWN)
            {
                Menu_Index++;

                // 1. 判断是否超出总菜单数（循环回顶部）
                if (Menu_Index >= MENU_ITEM_COUNT)
                {
                    Menu_Index = 0;  // 光标回到第一项
                    Show_Offset = 0; // 屏幕窗口也回到最顶部
                }
                // 2. 判断光标是否超出了当前屏幕显示的底部
                else if (Menu_Index >= Show_Offset + PAGE_SIZE)
                {
                    // 屏幕窗口整体向下滚动一行
                    Show_Offset++;
                }

                // 重新绘制菜单界面
                OLED_DrawMenu();
            }
            else if (recv_event == UI_EVENT_KEY1_DOWN)
            {
                Execute_Menu_Action(Menu_Index);
            }
        }
        else if (cur_page == Page_DHT11_Data)
        {
            // 如果在传感器界面，收到了退出按键
            if (recv_event == UI_EVENT_KEY1_DOWN)
            {
                cur_page = menuPage;
                OLED_Clear();
                OLED_DrawMenu();
            }
            // 如果在传感器界面，收到了传感器任务发来的“数据更新”通知
            else if (recv_event == UI_EVENT_DHT11_READY)
            {
                // 直接从全局变量取极速渲染，没有任何延时！
                OLED_ShowNum(64, 2, g_SensorData.temp, 2, 16, 0);
                OLED_ShowNum(64, 4, g_SensorData.humi, 2, 16, 0);
            }
        }
        else if (cur_page == Page_RGBControl)
        {
            if (recv_event == UI_EVENT_KEY0_DOWN) // 光标下移
            {
                RGB_Menu_Index++;
                if (RGB_Menu_Index >= 4) // 只有4行，超出则回到顶部
                {
                    RGB_Menu_Index = 0;
                }
                OLED_DrawRGBMenu();
            }
            else if (recv_event == UI_EVENT_KEY1_DOWN) // 确认翻转状态
            {
                Execute_RGB_Action();
            }
        }
		else if (cur_page == Page_MPU6050)
		{
			if (recv_event == UI_EVENT_KEY1_DOWN) 
            {
                // 短按 KEY1：切换显示模式 (0和1之间翻转)
                mpu_display_mode = !mpu_display_mode; 
                OLED_DrawMPU6050Page(); // 立即刷新一帧响应按键
            }
            else if (recv_event == UI_EVENT_MPU6050_READY)
            {
                // 收到传感器任务发来的新数据，刷新屏幕
                OLED_DrawMPU6050Page();
            }
		}
		else if (cur_page == Page_RTC)
		{
			if (recv_event == UI_EVENT_RTC_READY)
			{
				OLED_DrawRTCPage();
			}
		}
        else if (cur_page == lowPowerPage)
        {
            if (recv_event == UI_EVENT_KEY1_DOWN)
            {
                Execute_LowPower_Toggle();
            }
        }
        else
        {
            if (recv_event == UI_EVENT_KEY1_DOWN)
            {
                cur_page = menuPage;
                OLED_Clear();
                OLED_DrawMenu();
            }
        }
    }
}


