#include "stm32f10x.h"
#include <stdint.h>

#ifndef __SERIAL_H_
#define __SERIAL_H_

#define USART1_BUFF_SIZE 500

extern uint8_t USART2_RxFlag;
extern char USART2_Packet[500];

void USART1_init(uint32_t BaudRate);
void USART2_init(uint32_t BaudRate);
void USART_SendByte(USART_TypeDef *USARTx, uint8_t Byte);
void USART1_SendString(char *String);
void USART2_SendString(char *String);

#endif
