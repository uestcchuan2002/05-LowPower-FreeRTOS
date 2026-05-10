#ifndef __UI_GLOBAL_H   
#define __UI_GLOBAL_H     

#include <oled.h>
#include <stdio.h>
#include "DHT11.h"
#include "app_global.h"

#define MENU_ITEM_COUNT 7  // 你的菜单总项数，可任意增加
#define PAGE_SIZE       4  // 屏幕一页最多显示的行数 (对于16号字体是4行)

typedef enum  {
    menuPage = 0,
    Page_DHT11_Data,
    Page_RGBControl,
    Page_MPU6050,
    Page_RTC,
    Page5,
    lowPowerPage,
    Page7
} currentPage;



typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_KEY0_DOWN, 
    UI_EVENT_KEY1_DOWN,
    UI_EVENT_KEY1_LONG,
    UI_EVENT_DHT11_READY,   // DHT11 有新数据了
    UI_EVENT_MPU6050_READY,  // MPU6050 有新数据了
	UI_EVENT_RTC_READY      //  新增：RTC 时间更新事件
} UI_Event_t;

extern currentPage cur_page;
extern osMessageQueueId_t UI_QueueHandle;
extern uint8_t g_LowPowerMode;

extern uint8_t led_state_white;
extern uint8_t led_state_red;
extern uint8_t led_state_blue;
extern uint8_t led_state_green;
extern uint8_t RGB_Menu_Index;

void AppTask_Gui(void *argument);

void OLED_DrawMenu(void);
void Execute_Menu_Action(uint8_t index);

void DHT11_Data_Ui(uint8_t g_SensorEnable);  

void RGB_Contorl_Ui(void);
void OLED_DrawRGBMenu(void);
void Execute_RGB_Action(void);

void OLED_DrawLowPowerPage(void);
void Execute_LowPower_Toggle(void);

void OLED_DrawMPU6050Page(void);

void OLED_DrawRTCPage(void);

#endif 
