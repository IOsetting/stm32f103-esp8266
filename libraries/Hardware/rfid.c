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
  // Delay 50ms to avoid being joinned by other commands
  Systick_Delay_ms(50);

  u8 waittime = 10;
  while (waittime--) {
    if(RFID_RX_STATE > 0) {
      printf("RFID_RX_STATE %d\r\n", RFID_RX_STATE);
      if (Buffer_Pop_All(&RFID_RX_BUF, &RFID_RX_CLIP) != NULL) {
        Buffer_Clip_Print_Hex(&RFID_RX_CLIP);
        RFID_Handle_Message();
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

u8 RFID_Checksum(u8 *buff, u8 len)
{
	u8 usum = 0;
	for(u8 i = 0; i < len; i++) {
		usum = usum + buff[i];
	}
	return usum;
}

void RFID_Handle_Message(void)
{
  // step - 0:init, 1:Header 0xAA, 2:Type 0x??, 3:Command 0x??, 4:Length 0x????, 5:Params ..., 6:Checksum 0x?? 7:End 0xDD
  u8 pos = 0, step = 0, command = 0x00, length = 0, dummy = 0x00;
  u8 checksum_start = 0, param_start = 0;
  char tmp[3];
  while (pos < RFID_RX_CLIP.length) {
    switch (step) {
      case 0: // init-> Header 0xAA
        command = 0x00;
        length = 0;
        checksum_start = 0;
        param_start = 0;
        if (RFID_RX_CLIP.data[pos] == 0xAA) {
          pos++;
          step = 1;
        } else {
          pos++;
        }
        break;
      case 1: // Header -> Type 0x??
        if (RFID_RX_CLIP.data[pos] == 0x01 || RFID_RX_CLIP.data[pos] == 0x02) {
          checksum_start = pos; // Mark the checksum calcu start point
          pos++;
          step = 2;
        } else {
          step = 0;
          continue;
        }
        break;
      case 2: // Type -> Command 0x??
        command = RFID_RX_CLIP.data[pos];
        switch (command) {
          case 0xFF: // Error
            break;
          case 0x03: // 获取读写器模块信息
            break;
          case 0x22: // 单次轮询, 多次轮询
            break;
          case 0x28: // 停止多次轮询
            break;
          case 0x0C: // 设置Select参数, 设置Select模式
            break;
          case 0x0B: // 获取Select参数
            break;
          case 0x39: // 读标签数据存储区
            break;
          case 0x49: // 写标签数据存储区
            break;
          case 0x82: // 锁定Lock标签数据存储区
            break;
          case 0x65: // Kill标签
            break;
          case 0x0D: // 获取Query参数
            break;
          case 0x0E: // 设置Query参数
            break;
          case 0x07: // 设置工作地区
            break;
          case 0x08: // 获取工作地区
            break;
          case 0xAB: // 设置工作信道
            break;
          case 0xAA: // 获取工作信道
            break;
          case 0xAD: // 设置自动跳频
            break;
          case 0xA9: // 插入工作信道
            break;
          case 0xB7: // 获取发射功率
            break;
          case 0xB6: // 设置发射功率
            break;
          case 0xB0: // 设置发射连续载波
            break;
          case 0xF1: // 获取接收解调器参数
            break;
          case 0xF0: // 设置接收解调器参数
            break;
          case 0xF2: // 测试射频输入端阻塞信号
            break;
          case 0xF3: // 测试信道RSSI
            break;
          case 0x1A: // 控制IO端口
            break;
          case 0x17: // 模块休眠
            break;
          case 0x1D: // 模块空闲休眠时间
            break;
          case 0x04: // 模块 ILDE 模式
            break;
          case 0xE1: // NXP ReadProtect指令
            break;
          case 0xE2: // NXP Reset ReadProtect指令
            break;
          case 0xE3: // NXP Change EAS指令
            break;
          case 0xE4: // NXP EAS_Alarm指令
            break;
          case 0xE0: // NXP ChangeConfig指令
            break;
          case 0xE5: // Impinj Monza QT指令, Read/Write 数据域为 0x00时
            break;
          case 0xE6: // Impinj Monza QT指令, Read/Write 数据域为 0x01时
            break;
          case 0xD3: // BlockPermalock指令, Read/Lock 数据域为 0x00时
            break;
          case 0xD4: // BlockPermalock指令, Read/Lock 数据域为 0x01时
            break;
          default:   // 未知响应
            break;
        }
        pos++;
        step = 3;
        break;
      case 3: // Command -> Length 0x????
        if (pos + 1 < RFID_RX_CLIP.length) {
          length = RFID_RX_CLIP.data[pos + 1];
          pos += 2;
          step = 4;
        } else {
          step = 0;
          continue;
        }
        break;
      case 4: // Length -> Params
        if (pos + length < RFID_RX_CLIP.length) {
          param_start = pos;
          pos += length;
          step = 5;
        } else {
          step = 0;
          continue;
        }
        break;
      case 5:
        dummy = RFID_Checksum(RFID_RX_CLIP.data + checksum_start, pos - checksum_start);
        if (dummy == RFID_RX_CLIP.data[pos]) {
          pos++;
          step = 6;
        } else {
          printf("Checksum failed %02X, expect %02X\r\n", dummy, RFID_RX_CLIP.data[pos]);
          step = 0;
          continue;
        }
        break;
      case 6: // Checksum -> End 0xDD
        printf("ps:%d, len:%d: ", param_start, length);
        sprintf(tmp, "%02X:", command);
        ESP8266_Send_Data((u8*)tmp, 3);
        for (u8 i = param_start; i < param_start + length; i++) {
          sprintf(tmp, "%02X ", *(RFID_RX_CLIP.data + i));
          printf("%s", tmp);
          ESP8266_Send_Data((u8*)tmp, 3);
        }
        printf("\r\n");
        ESP8266_Send_Data((u8*)"\r\n", 2);
        if (RFID_RX_CLIP.data[pos] == 0xDD) {
          pos++;
        }
        step = 0;
        break;
    }
  }
}
