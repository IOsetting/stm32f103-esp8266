# stm32f103-esp8266
A working demo of ESP8266-S01 UART module running on STM32F103C8T6

# Components Required

* STM32F103C8 board
* ESP-01S: ESP8266 Wi-Fi module
* USB2TTL: PL2303 or CH340 or FT232 or CP2102
* ST-Link or J-LInk
* Laptop & Wi-Fi hotspot

# Circuit Diagram and connections

Refer below table to connect ESP-01S pins with STM32 pins:

| ESP-01S   | STM32    | USB2TTL      |
| --------- | -------- | ------------ |
| GND       | GND      | GND          |
| TX(GPIO1) | PA10     |              |
| RX(GPIO3) | PA9      |              |
| 3.3V      | 3.3V     |              |
|           | PA3      | TX           |
|           | PA2      | RX           |

# Working & Code Explanation

The working of interfacing ESP8266 with STM32 is very simple. I use ST-Link to upload the code to STM32.  

## Compile Control String

```bash
--c99 --gnu -c --cpu Cortex-M3 -g -O0 --apcs=interwork --split_sections -I ../Libraries/CMSIS/CM3/CoreSupport -I ../Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x -I ../Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/arm -I ../Libraries/STM32F10x_StdPeriph_Driver/inc -I ../Libraries/Hardware -I ../user
-I./RTE/_stm32f103esp8266
-IC:/Keil_v5/ARM/PACK/Keil/STM32F1xx_DFP/2.3.0/Device/Include
-IC:/Keil_v5/ARM/CMSIS/Include
-D__UVISION_VERSION="525" -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DPRINT_TO_UART
-o .\Objects\*.o --omf_browse .\Objects\*.crf --depend .\Objects\*.d
```