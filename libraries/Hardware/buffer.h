#ifndef __BUFFER_H_
#define __BUFFER_H_

#include "stm32f10x.h"

#pragma pack(1)
typedef struct
{
  u16* buf;
  u16 size;
  u16 front;
  u16 rear;
} BufferTypeDef;
#pragma pack()

void Buffer_Init(BufferTypeDef* buff, u16 size);
void Buffer_Push(BufferTypeDef* buff, u16 data);
u16 Buffer_Pop(BufferTypeDef* buff);
void Buffer_Print(BufferTypeDef* buff);
void Buffer_PrintHex(BufferTypeDef* buff);
void Buffer_Print_All(BufferTypeDef* buff);

#endif
