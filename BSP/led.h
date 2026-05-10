#ifndef __LED_H
#define __LED_H
 
#include "stm32f1xx_hal.h"
#include "main.h"

#define LED0(x) x == 0 ?  \
													HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin,  GPIO_PIN_SET) \
													: HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin,  GPIO_PIN_RESET);

void LED0_Init(void);
void LED0_Troggle(void);
void LED_RGB_Init(void);
void LED_RGB_White(uint8_t state);
void LED_RGB_Red(uint8_t state); 
void LED_RGB_Blue(uint8_t state);
void LED_RGB_Green(uint8_t state);
void LED_RGB_Set(uint8_t r, uint8_t g, uint8_t b);
#endif 


