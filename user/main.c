#include "include.h"
#include "config.h"

int main(void)
{
  SystemInit();
  Systick_Init();

  USART1_Init();  //日志打印串口初始化
  //TIM1_Init();
  //TIM2_Init();
  TIM3_Init();
  PB12_Init();
  ESP8266_Init();
  printf("## Init finished ##\r\n");
  ESP8266_Set_Stationmode();
  printf("## Switched to station mode ##\r\n");

  if(ESP8266_Connect_AP(WIFI_SSID, WIFI_PASSWORD) != ACK_SUCCESS) {
    printf("## Connect to WiFi failed. ##\r\n");
    while(1)
    {;}
  }
  printf("## WiFi connected ##\r\n");

  if (ESP8266_Set_Link_Mux(0) != ACK_SUCCESS) {
    printf("## Set MUX failed. ##\r\n");
    while(1)
    {;}
  }
  printf("## MUX Set To 0 ##\r\n");

  const char* address = "192.168.6.210";
  const char* port = "3333";
  if(ESP8266_Connect_TCP(address, port) != ACK_SUCCESS) {
    printf("## Connect to %s failed. ##\r\n", address);
    while(1)
    {;}
  }
  printf("## Connected to %s ##\r\n", address);

  //ESP8266_Send_Cmd("AT+CIPCLOSE\r\v", "CLOSED", 20);  //断开连接
  u16 count = 0;
  while(1) {
    char request[128];
    sprintf(request, "This is No.%d request. 设置为透传, 透传不允许指定发送长度, 此时从远程传入的信息前面不带\r\n", count++);
    if (ESP8266_Start_Passthrough() != ACK_SUCCESS) {
      printf("## Switch to passthrough failed ##\r\n");
      if(ESP8266_Connect_TCP(address, port) != ACK_SUCCESS) {
        printf("Connect to %s failed.\r\n", address);
      } else {
        printf("Reconnect to %s.\r\n", address);
      }
    } else {
      PB12_On();
      Passthrough_Echo_Test(request);
      PB12_Off();
      ESP8266_Quit_Passthrough();
    }
    //ESP8266_Send_Cmd("+++\r\n", "OK", 50);
    //ESP8266_Send_Cmd("AT\r\n", "OK", 50);
    //ESP8266_Passthrough_Request(UDP, "192.168.6.210", "3333", Passthrough_Echo_Test);
    Systick_Delay_ms(1000);
  }
}



