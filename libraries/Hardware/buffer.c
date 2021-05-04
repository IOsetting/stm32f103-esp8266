#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

/**
 * Buffer Utilities
*/
void Buffer_Init(BufferTypeDef* buff, u16 size)
{
  buff->size = size;
  buff->buf = malloc( size * sizeof(unsigned char));
  memset(buff->buf, 0, size * sizeof(unsigned char));
  buff->front = 0;
  buff->rear = 0;
}

void Buffer_Push(BufferTypeDef* buff, u16 data)
{
  buff->buf[buff->rear] = data;
  buff->rear++;
  if (buff->rear >= buff->size) {
    buff->rear = 0;
  }
  if (buff->front == buff->rear) {
    buff->front++;
    if (buff->front >= buff->size) {
      buff->front = 0;
    }
  }
}

u16 Buffer_Pop(BufferTypeDef* buff)
{
  if (buff->front == buff->rear) return NULL;

  u16 data = buff->buf[buff->front];
  buff->front++;
  if (buff->front >= buff->size) {
    buff->front = 0;
  }
  return data;
}

void Buffer_Print(BufferTypeDef* buff)
{
  printf("BUFF:[%03d,%03d)",buff->front, buff->rear);
  if (buff->front == buff->rear) {
    // print nothing;
  } else if (buff->front < buff->rear) {
    for(int i=buff->front; i < buff->rear; i++) {
      printf("%c", buff->buf[i]);
    }
  } else {
    for(int i = buff->front; i < buff->size; i++) {
      printf("%c", buff->buf[i]);
    }
    for(int i = 0; i < buff->rear; i++) {
      printf("%c", buff->buf[i]);
    }
  }
  printf("\r\n");
}

void Buffer_PrintHex(BufferTypeDef* buff)
{
  printf("BUFF:[%03d,%03d)",buff->front, buff->rear);
  if (buff->front == buff->rear) {
    // print nothing;
  } else if (buff->front < buff->rear) {
    for(int i=buff->front; i<buff->rear; i++) {
      printf("%02X ", buff->buf[i]);
    }
  } else {
    for(int i=buff->front; i < buff->size; i++) {
      printf("%02X ", buff->buf[i]);
    }
    for(int i=0; i<buff->rear; i++) {
      printf("%02X ", buff->buf[i]);
    }
  }
  printf("\r\n");
}

void Buffer_Print_All(BufferTypeDef* buff)
{
  printf("BUFF:[%d,%d)",buff->front, buff->rear);
  for(int i=0; i < buff->size; i++) {
    printf("%c", buff->buf[i]);
  }
  printf("\r\n");
}
