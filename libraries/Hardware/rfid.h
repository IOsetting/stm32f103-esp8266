#ifndef __RFID_H_
#define __RFID_H_

#include <stdbool.h>
#include "stm32f10x.h"

void RFID_Init(void);
u16  RFID_Pop_Ack(u8* buff_tmp, u16 limit);
void RFID_Send_String(const u8* data, u16 length);
bool RFID_Send_Cmd(const u8* cmd, u16 length);
u8   RFID_Checksum(u8 *buff, u8 len);

void RFID_Check_Version(void);
void RFID_Query_Config(void);
void RFID_Inventory_Single(void);

void RFID_Handle_Message(void);



#endif
