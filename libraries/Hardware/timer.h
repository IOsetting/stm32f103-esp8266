#ifndef __TIMER_H_
#define __TIMER_H_

#include "stm32f10x.h"

void Systick_Init(void);
void Systick_Delay_ms(__IO uint32_t count);

void TIM1_Init(void);

void TIM2_Init(void);
void TIM2_Delay_10us(__IO uint32_t count);
void TIM2_Delay_ms(__IO uint32_t count);

void TIM3_Init(void);

#endif
