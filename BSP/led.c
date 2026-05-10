#include "led.h"

void LED0_Init(void) {
	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin,  GPIO_PIN_RESET);
}

void LED0_Troggle(void) {
	HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
}
 
void LED_RGB_Init(void) 
{
    // 电源开启
    HAL_GPIO_WritePin(LED_R_Power_GPIO_Port, LED_POWER_Pin,  GPIO_PIN_SET);
    // 其余RGB高电平，不导通
    HAL_GPIO_WritePin(LED_R_Power_GPIO_Port, LEDR_Pin,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_G_GPIO_Port, LEDB_Pin,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_G_GPIO_Port, LEDG_Pin,  GPIO_PIN_SET);
}

/**
 * @description: RGB 底层驱动封装 (内部使用)
 * @param {uint8_t} r, g, b: 传入 1 代表亮，0 代表灭
 */
void LED_RGB_Set(uint8_t r, uint8_t g, uint8_t b)
{
    // 你的硬件是低电平(RESET)点亮，高电平(SET)熄灭
    // 使用三目运算符 ?: 直接映射状态，避免写一堆 if-else
    HAL_GPIO_WritePin(LED_R_Power_GPIO_Port, LEDR_Pin, r ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_G_GPIO_Port,     LEDG_Pin, g ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_G_GPIO_Port,     LEDB_Pin, b ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

// ==========================================
// 对外提供的应用层 API，极其精简
// ==========================================

void LED_RGB_White(uint8_t state)
{
    // 白光：RGB 全亮 (1,1,1) 或全灭 (0,0,0)
    LED_RGB_Set(state, state, state);
}

void LED_RGB_Red(uint8_t state)
{
    // 红光：只有 R 跟随 state 变化，G 和 B 强制熄灭 (0)
    LED_RGB_Set(state, 0, 0);
}

void LED_RGB_Green(uint8_t state)
{
    // 绿光：只有 G 跟随 state 变化，R 和 B 强制熄灭 (0)
    LED_RGB_Set(0, state, 0);
}

void LED_RGB_Blue(uint8_t state)
{
    // 蓝光：只有 B 跟随 state 变化，R 和 G 强制熄灭 (0)
    LED_RGB_Set(0, 0, state);
}
