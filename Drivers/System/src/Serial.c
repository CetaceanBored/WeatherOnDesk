#include "stm32f10x.h"
#include "TIM.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Serial.h"


char USART1_TX_BUFF[USART1_BUFF_SIZE];
char USART1_RX_BUFF[USART1_BUFF_SIZE];
volatile uint16_t USART1_RX_Index = 0;
char USART2_Packet[500];
uint8_t USART2_RxFlag;


void USART1_init(uint32_t BaudRate)      //USART1------PA9 = TX  PA10 = RX
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);

	USART_Cmd(USART1, ENABLE);
	TIM3_Init();
}

void USART2_init(uint32_t BaudRate)      //USART2------PA2 = TX  PA3 = RX
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStruct);
    
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);

	USART_Cmd(USART2, ENABLE);
}

void USART_SendByte(USART_TypeDef *USARTx, uint8_t Byte)
{
    USART_SendData(USARTx, Byte);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

void USART2_SendString(char *String)
{
	for (uint8_t i = 0; String[i] != '\0'; i ++) {
		USART_SendByte(USART2, String[i]);
	}
}

void USART1_SendString(char *String)
{
	memset(USART1_TX_BUFF, 0, sizeof(USART1_TX_BUFF));
	sprintf(USART1_TX_BUFF, "%s", String);
	uint16_t len = strlen(USART1_TX_BUFF);
	for (uint16_t j = 0; j < len; j++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, USART1_TX_BUFF[j]);
	}
}

/*
@brief ESP8266 Receive Packet
@details
USART1_RX_Index: [15]: Packet receive complete or not, [14~0]: Packet Length
TIM3: If time of receiving two byte is over 10ms, consider the packet receive complete
*/
void USART1_IRQHandler(void)
{
	uint8_t RxData;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
		RxData = USART_ReceiveData(USART1);
		if ((USART1_RX_Index & (1 << 15)) == 0) {
			if (USART1_RX_Index < USART1_BUFF_SIZE) {
				TIM_SetCounter(TIM3, 0);
				if (USART1_RX_Index == 0) TIM_Cmd(TIM3, ENABLE);
				USART1_RX_BUFF[USART1_RX_Index++] = RxData;
			}
			else {
				USART1_RX_Index |= (1 << 15);
			}
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
		USART1_RX_Index |= (1 << 15);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		TIM_Cmd(TIM3, DISABLE);
	}
}

/*
@brief JDY-31 Receive Packet
@note END OF PACKET: '@'
*/
void USART2_IRQHandler(void)
{
	static uint8_t RxState = 0, pRxPacket = 0;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
		uint8_t RxData = USART_ReceiveData(USART2);
		if(RxState == 0) {
			if (USART2_RxFlag == 0) {
				RxState = 1;
				pRxPacket = 0;
				USART2_Packet[pRxPacket++] = RxData;
			}
		}
		else {
			if (RxData == '@') {
				RxState = 0;
				USART2_RxFlag = 1;
				USART2_Packet[pRxPacket] = '\0';
			}
			else {
				USART2_Packet[pRxPacket++] = RxData;
			}
		}
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}

