#include "stm32f10x.h"
#include "Delay.h"

#define DHT11_DQ_GPIO_PORT			GPIOA
#define DHT11_DQ_GPIO_PIN			GPIO_Pin_1
#define DHT11_DQ_GPIO_CLK_ENABLE() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

#define DHT11_DQ_OUT(x)				GPIO_WriteBit(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN, x)
#define DHT11_DQ_IN 				GPIO_ReadInputDataBit(DHT11_DQ_GPIO_PORT, DHT11_DQ_GPIO_PIN)

void DHT11_reset()
{
	DHT11_DQ_OUT(0);
	Delay_ms(20);
	DHT11_DQ_OUT(1);
	Delay_us(30);
}

uint8_t DHT11_check()
{
	uint8_t retry = 0;
	uint8_t rval = 0;
	while(DHT11_DQ_IN && retry < 100) {
		retry++;
		Delay_us(1);
	}
	if(retry >= 100) {
		rval = 1;
	}
	else {
		retry = 0;
		while(!DHT11_DQ_IN && retry < 100) {
			retry++;
			Delay_us(1);
		}
		if(retry >= 100) {
			rval = 1;
		}
	}
	return rval;
}

uint8_t DHT11_init()
{
	DHT11_DQ_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStucture;
	GPIO_InitStucture.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStucture.GPIO_Pin = DHT11_DQ_GPIO_PIN;
	GPIO_InitStucture.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT11_DQ_GPIO_PORT, &GPIO_InitStucture);
	
	DHT11_reset();
	return DHT11_check();
}

uint8_t DHT11_ReadBit(void)
{
	uint8_t retry = 0;
	while(DHT11_DQ_IN && retry < 100) {
		retry++;
		Delay_us(1);
	}
	retry = 0;
	while(!DHT11_DQ_IN && retry < 100) {
		retry++;
		Delay_us(1);
	}
	Delay_us(40);
	if(DHT11_DQ_IN)	return 1;
	else	return 0;
}

uint8_t DHT11_ReadByte(void)
{	uint8_t i, Data = 0;
	for (i = 0; i < 8; i++) {
		Data <<= 1;
		Data |= DHT11_ReadBit();
	}
	return Data;
}

uint8_t DHT11_ReadData(uint8_t *T, uint8_t *H)
{
	uint8_t i, buf[5];
	DHT11_reset();
	if(DHT11_check() == 0) {
		for(i = 0; i < 5; i++) {
			buf[i] = DHT11_ReadByte();
		}
		if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
			*T = buf[2];
			*H = buf[0];
		}
	}
	return 0;
}