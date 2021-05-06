#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

void Buffer_Reset(BufferTypeDef* buff)
{
  buff->front = 0;
  buff->rear = 0;
}

u16 Buffer_Length(BufferTypeDef* buff)
{
  if (buff->rear >= buff->front) {
    return buff->rear - buff->front;
  } else {
    return (buff->size - buff->front) + (buff->rear - 0);
  }
}

u8 Buffer_Push(BufferTypeDef* buff, u8 data)
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
    return NULL;
  } else {
    return !NULL;
  }
}

u8 Buffer_Pop(BufferTypeDef* buff, u8* data)
{
  if (buff->front == buff->rear) return NULL;

  *data = buff->buf[buff->front];
  buff->front++;
  if (buff->front >= buff->size) {
    buff->front = 0;
  }
  return !NULL;
}

u8 Buffer_Pop_All(BufferTypeDef* buff, BufferClip* clip)
{
  if (buff->front == buff->rear) return NULL;
  
  memset(clip->data, 0x00, clip->size * sizeof(u8));
  clip->length = 0;
  if (buff->front > buff->rear) {
    while (buff->front < buff->size && clip->length <= clip->size) {
      *(clip->data + clip->length++) = buff->buf[buff->front++];
    }
    if (buff->front == buff->size) {
      buff->front = 0;
    }
  }
  while (buff->front < buff->rear && clip->length <= clip->size) {
    *(clip->data + clip->length++) = buff->buf[buff->front++];
  }
  return !NULL;
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

void Buffer_Print_Hex(BufferTypeDef* buff)
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

void Buffer_Clip_Print(BufferClip* clip)
{
  printf("CLIP:[%03d]", clip->length);
  for(int i = 0; i < clip->length; i++) {
    printf("%c", clip->data[i]);
  }
  printf("\r\n");
}

void Buffer_Clip_Print_Hex(BufferClip* clip)
{
  printf("CLIP:[%03d]", clip->length);
  for(int i = 0; i < clip->length; i++) {
    printf("%02X ", clip->data[i]);
  }
  printf("\r\n");
}
