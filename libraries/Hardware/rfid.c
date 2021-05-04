#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "rfid.h"
#include "timer.h"

#define RFID_RCC       RCC_APB1Periph_USART3
#define RFID_TX_PIN    GPIO_Pin_10   //B10
#define RFID_TX_GPIO   GPIOB
#define RFID_RX_PIN    GPIO_Pin_11   //B11
#define RFID_RX_GPIO   GPIOB
#define RFID_USART     USART3

BufferTypeDef RFID_RX_BUF;
u8 RFID_RX_STATE = 0;


void RFID_Init(void)
{
  Buffer_Init(&RFID_RX_BUF, RFID_BUF_SIZE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RFID_RCC, ENABLE);
  // GPIO for TX
  GPIO_InitTypeDef  GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = RFID_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(RFID_TX_GPIO, &GPIO_InitStructure);
  // GPIO for RX
  GPIO_InitStructure.GPIO_Pin = RFID_RX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(RFID_RX_GPIO, &GPIO_InitStructure);	
  // NVIC
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  // USART
  USART_DeInit(RFID_USART);
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(RFID_USART, &USART_InitStructure);
  USART_ClearFlag(RFID_USART, USART_FLAG_CTS);
  USART_Cmd(RFID_USART, ENABLE);

  USART_ITConfig(RFID_USART, USART_IT_RXNE, ENABLE);
  printf("## UHF RFID Initialized ##\r\n");
}

void USART3_IRQHandler(void)
{
  unsigned char rev_byte;
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  {
    rev_byte = USART_ReceiveData(USART3);
    Buffer_Push(&RFID_RX_BUF, rev_byte);
    // Reset the TIM2 counter and enable it
    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
  }
}

void TIM2_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
    printf("RFID_RX_STATE++\r\n");
    RFID_RX_STATE++;
  }
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  TIM_Cmd(TIM2, DISABLE);
}
