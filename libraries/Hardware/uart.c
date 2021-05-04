#include <stdio.h>
#include "uart.h"

#ifdef PRINT_TO_UART

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  while(USART_GetFlagStatus(PRINT_USARTX, USART_FLAG_TXE) == RESET){}
  USART_SendData(PRINT_USARTX, ch);
  return(ch);
}

int fgetc(FILE *f) {
  char ch;
  while(USART_GetFlagStatus(PRINT_USARTX, USART_FLAG_RXNE) == RESET){}
  ch = USART_ReceiveData(PRINT_USARTX);
  return((int)ch);
}

int ferror(FILE *f) {
  return EOF;
}

void _ttywrch(int ch) {
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
  USART_SendData(PRINT_USARTX, ch);
}

void _sys_exit(int return_code) {
  while (1); /* endless loop */
}
#endif


/**
 * UART1: AP9, AP10
 */
void USART1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  // 打开GPIO和USART时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  // 将GPIO配置为推挽复用模式
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  // 将GPIO配置为浮空输入模式
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);	

  /* 配置USART参数
      波特率   = 115200
      数据长度 = 8
      停止位   = 1
      校验位   = No
      禁止硬件流控(即禁止RTS和CTS)
      使能接收和发送
  */
  USART_DeInit(USART1);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART1, &USART_InitStructure);
  USART_ClearFlag(USART1, USART_FLAG_CTS);
  USART_Cmd(USART1, ENABLE);
  printf("## UART1 Initialized ##\r\n");
}

#ifdef WITH_UART2
/**
 * UART2: AP2, AP3
 */
void USART2_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  // 打开GPIO和USART时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  // 将GPIO配置为推挽复用模式
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  // 将GPIO配置为浮空输入模式
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);	

  /* 配置USART参数
      波特率   = 115200
      数据长度 = 8
      停止位   = 1
      校验位   = No
      禁止硬件流控(即禁止RTS和CTS)
      使能接收和发送
  */
  USART_DeInit(USART2);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART2, &USART_InitStructure);
  USART_ClearFlag(USART2, USART_FLAG_CTS);
  USART_Cmd(USART2, ENABLE);
  printf("## UART2 Initialized ##\r\n");
}

void USART2_NVIC_Init(void)
{
  NVIC_InitTypeDef NVIC_InitStruct;
  // fill in the interrupt configuration
  NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

/*串口接收中断函数*/	
void USART2_IRQHandler(void)
{
  unsigned char rev_byte;
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)	{
    rev_byte = USART_ReceiveData(USART2);
    printf("%c\r\n", rev_byte);
  }
  USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}	
#endif
