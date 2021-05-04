#include <stdio.h>
#include "pb12.h"

void PB12_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  printf("## PB12 Initialized ##\r\n");
}

void PB12_Off(void)
{
  GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

void PB12_On(void)
{
  GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}
