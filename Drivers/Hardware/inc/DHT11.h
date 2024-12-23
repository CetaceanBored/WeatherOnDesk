#include "stm32f10x.h"

#ifndef __DTH11_H_
#define	__DTH11_H_

void DHT11_reset();
uint8_t DHT11_check();
uint8_t DHT11_init();
uint8_t DHT11_ReadBit(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadData(uint8_t *T, uint8_t *H);

#endif
