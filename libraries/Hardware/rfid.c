#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "esp8266.h"
#include "rfid.h"
#include "timer.h"

#define RFID_RCC       RCC_APB1Periph_USART3
#define RFID_TX_PIN    GPIO_Pin_10   //B10
#define RFID_TX_GPIO   GPIOB
#define RFID_RX_PIN    GPIO_Pin_11   //B11
#define RFID_RX_GPIO   GPIOB
#define RFID_USART     USART3
#define RFID_BUF_SIZE  256

const u8 RFID_CMD_VERSION[]          = {0xAA, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0xDD};
const u8 RFID_CMD_QUERY_CONFIG[]     = {0xAA, 0x00, 0x0E, 0x00, 0x02, 0x10, 0x40, 0x60, 0xDD};
const u8 RFID_CMD_INVENTORY_SINGLE[] = {0xAA, 0x00, 0x22, 0x00, 0x00, 0x22, 0xDD};
const u8 RFID_CMD_INVENTORY_MULTI[]  = {0xAA, 0x00, 0x27, 0x00, 0x03, 0x22, 0x27, 0x10, 0x83, 0xDD};
// single x4: AA 00 0E 00 02 10 20 40 DD 
// single x8: AA 00 0E 00 02 10 40 60 DD    return AA 01 0E 00 01 00 10 DD
// read single: AA 00 22 00 00 22 DD    return AA 01 FF 00 01 15 16 DD
/*
regular return
AA 01 03 00 0F 00 55 48 46 20 32 36 64 42 6D 20 56 31 2E 31 97 DD 
AA 01 03 00 08 01 56 38 2E 32 36 2E 30 8F DD 
AA 01 03 00 0F 00 55 48 46 20 32 36 64 42 6D 20 56 31 2E 31 97 DD 
AA 01 03 00 08 01 56 38 2E 32 36 2E 30 8F DD 
AA 01 03 00 0F 00 55 48 46 20 32 36 64 42 6D 20 56 31 2E 31 97 DD 
AA 01 03 00 08 01 56 38 2E 32 36 2E 30 8F DD 
AA 01 03 00 0F 00 55 48 46 20 32 36 64 42 6D 20 56 31 2E 31 97 DD 
AA 01 03 00 08 01 56 38 2E 32 36 2E 30 8F DD 
AA 01 03 00 0F 00 55 48 46 20 32 36 64 42 6D 20 56 31 2E 31 97 DD 
AA 01 03 00 08 01 56 38 2E 32 36 2E 30 8F DD 
*/


BufferTypeDef RFID_RX_BUF;
u8 RFID_RX_BUF_BUFF[RFID_BUF_SIZE] = {0x00};

BufferClip RFID_RX_CLIP;
u8 RFID_RX_CLIP_DATA[UINT8_MAX] = {0x00};

u8 RFID_RX_STATE = 0;

void RFID_Init(void)
{
  RFID_RX_BUF.buf = RFID_RX_BUF_BUFF;
  RFID_RX_BUF.size = RFID_BUF_SIZE;
  RFID_RX_CLIP.data = RFID_RX_CLIP_DATA;
  RFID_RX_CLIP.size = UINT8_MAX;

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
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
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
  u8 rev_byte;
  u32 clear;
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

void RFID_Send_String(const u8* data, u16 length)
{
  printf("RFID CMD: ");
  for (u16 i = 0; i < length; i++) {
    printf("%02X ", *(data + i));
    USART_SendData(RFID_USART, *(data + i));
    while(USART_GetFlagStatus(RFID_USART, USART_FLAG_TXE) == RESET) { // Wait till sent
      ;// Do nothing
    }
  }
  printf(">> Sent\r\n");
}

bool RFID_Send_Cmd(const u8* cmd, u16 length)
{
  RFID_Send_String(cmd, length);
  // Put a 50ms delay here to pevent from be joinned by other commands
  Systick_Delay_ms(50);

  u8 waittime = 10;
  while (waittime--) {
    if(RFID_RX_STATE > 0) {
      printf("RFID_RX_STATE %d\r\n", RFID_RX_STATE);
      if (Buffer_Pop_All(&RFID_RX_BUF, &RFID_RX_CLIP) != NULL) {
        Buffer_Clip_Print_Hex(&RFID_RX_CLIP);
        char tmp[3] = {0};
        for (u8 i = 0; i < RFID_RX_CLIP.length; i++) {
          sprintf(tmp, "%02X ", *(RFID_RX_CLIP.data + i));
          ESP8266_Send_Data((u8*)tmp, 3);
        }
        ESP8266_Send_Data((u8*)"\r\n", 2);
      }
      RFID_RX_STATE--;
    }
    Systick_Delay_ms(50);
  }
  return true;
}

void RFID_Check_Version(void)
{
  RFID_Send_Cmd(RFID_CMD_VERSION, 8);
}

void RFID_Query_Config(void)
{
  RFID_Send_Cmd(RFID_CMD_QUERY_CONFIG, 9);
}

void RFID_Inventory_Single(void)
{
  RFID_Send_Cmd(RFID_CMD_INVENTORY_SINGLE, 7);
}

