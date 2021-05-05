#ifndef __BUFFER_H_
#define __BUFFER_H_

#include "stm32f10x.h"

typedef struct
{
  u8* buf;
  u16 size;
  u16 front;
  u16 rear;
} BufferTypeDef;

void Buffer_Reset(BufferTypeDef* buff);
u16  Buffer_Length(BufferTypeDef* buff);
u8   Buffer_Push(BufferTypeDef* buff, u8 data);
u8   Buffer_Pop(BufferTypeDef* buff, u8* data);
void Buffer_Print(BufferTypeDef* buff);
void Buffer_PrintHex(BufferTypeDef* buff);
void Buffer_Print_All(BufferTypeDef* buff);

#endif
