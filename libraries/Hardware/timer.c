#include <stdio.h>
#include "timer.h"

__IO uint32_t TimingDelay;       // System Tick 延时函数计数
__IO uint32_t TIM2_TimingDelay;  // Timer2 延时函数计数


/**********************************************
 *
 * Delay with Systick Interupt
 *
 */

void Systick_Init(void)
{
  /* SystemFrequency / 1000 1ms 中断一次
   * SystemFrequency / 100000 10us 中断一次
   * SystemFrequency / 1000000 1us 中断一次
   */
  if(SysTick_Config(SystemCoreClock / 1000)) {
    printf("## !! Capture error ##\r\n");
    while(1);
  }
}

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00) {
    TimingDelay--;
  }
}

void SysTick_Handler(void)
{
  TimingDelay_Decrement();
}

void Systick_Delay_ms(__IO uint32_t count)
{
  TimingDelay = count;
  while (TimingDelay != 0);
}


/**********************************************
 *
 * TIM1 Timer
 *
 */

void TIM1_Init(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  TIM_DeInit(TIM1);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler =(SystemCoreClock / 2000); // 预分频, 预分频后为 72M/(72M/2K) = 2K (Hz)
  TIM_TimeBaseStructure.TIM_Period = (2000 - 1);                // Reload周期数, (1/2K) * 2K = 1s
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;          // 只有高级定时器可设置，重复计数。重复N次产生一次中断
  TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM1, TIM_FLAG_Update);
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_Cmd(TIM1, ENABLE); // 开启TIM1
  printf("## TIM1 Initialized ##\r\n");
}

//TIM1 中断处理方法
void TIM1_UP_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
    printf("TIM1\r\n");
  }
  TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}


/**********************************************
 *
 * TIM2 Timer
 *
 */

void TIM2_Init(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  TIM_DeInit(TIM2);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler =(SystemCoreClock / 200000); // -> 200KHz
  TIM_TimeBaseStructure.TIM_Period = 5000 - 1; // 200KHz -> 1/200K -> 1/200K * 5000 = 1/40 s = 25ms
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //TIM_Cmd(TIM2, ENABLE);
  printf("## TIM2 Initialized ##\r\n");
}


/**********************************************
 *
 * TIM3 Timer
 *
 */

void TIM3_Init(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  TIM_DeInit(TIM3);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler =(SystemCoreClock / 200000); // -> 200KHz
  TIM_TimeBaseStructure.TIM_Period = 5000 - 1; // 200KHz -> 1/200K -> 1/200K * 5000 = 1/40 s = 25ms
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  // This will be enabled in USART2_IRQHandler(), 
  // if it be enabled here, it will introduce an extra count 
  // TIM_Cmd(TIM3, ENABLE); 
  
  printf("## TIM3 Initialized ##\r\n");
}

