#include "include.h"
#include "config.h"

int main(void)
{
  SystemInit();
  Systick_Init();

  USART1_Init(); // for printf() logging
  TIM2_Init(); // TIM2 for UHF RFID
  TIM3_Init(); // TIM3 for ESP8266
  PB12_Init();
  PB12_Off(); // Turn off PB12 LED
  ESP8266_Init();
  RFID_Init();
  
  ESP8266_Set_Stationmode();
  printf("## Switched to station mode ##\r\n");

  if(ESP8266_Connect_AP(WIFI_SSID, WIFI_PASSWORD) != ACK_SUCCESS) {
    printf("## Connect to WiFi failed. ##\r\n");
    while(1)
    {;}
  }
  PB12_On(); // Turn on LED after wifi connected
  printf("## WiFi connected ##\r\n");

  if (ESP8266_Set_Link_Mux(0) != ACK_SUCCESS) {
    printf("## Set MUX failed. ##\r\n");
    while(1)
    {;}
  }
  printf("## MUX Set To 0 ##\r\n");

  const char* address = "192.168.6.210";
  const char* port = "3333";

  u16 count = 0;
  while(1) {
    char request[128];
    sprintf(request, "This is No.%d request. 设置为透传, 透传不允许指定发送长度, 此时从远程传入的信息前面不带\r\n", count++);
    if (ESP8266_Start_Passthrough() != ACK_SUCCESS) {
      printf("## Switch to passthrough failed ##\r\n");
      ESP8266_Send_Cmd("+++", "", 15);
      if(ESP8266_Connect_TCP(address, port) != ACK_SUCCESS) {
        printf("Connect to %s failed. Retry after 5 seconds\r\n", address);
        Systick_Delay_ms(5000);
      } else {
        printf("Reconnected to %s.\r\n", address);
      }
    } else {
      PB12_On();
      Passthrough_Echo_Test(request);
      
      ESP8266_Quit_Passthrough();
    }
    Systick_Delay_ms(1000);
  }
}




