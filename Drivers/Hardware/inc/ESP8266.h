#include "stm32f10x.h"

#ifndef __ESP8266_H_
#define __ESP8266_H_

void ESP8266_Init(void);
uint8_t ESP8266_SendCommand(char *Command, char *Response, uint32_t Timeout);
uint8_t ESP8266_GetData(char *Command, char *Response, uint16_t WaitTime);
uint8_t ESP8266_ConnectWiFi(void);
uint8_t ESP8266_StartTransparentTransmission(void);
uint8_t ESP8266_QuitTransparentTransmission(void);

#endif
