#include "stm32f10x.h"
#include "Delay.h"
#include "u8g2.h"
#include "Serial.h"
#include "OLED.h"
#include "ESP8266.h"
#include "weather.h"
#include "MQ-2.h"
#include "DHT11.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

u8g2_t U8g2;
uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr);
void u8g2_init(void);
void U8g2_Update(u8g2_t *u8g2);
void CheckESP8266Status(void);
char City[500], Message[500];
uint8_t Temperature, Humidity, Smoke;
uint8_t WeatherFlag;
extern char WeatherText[20], WeatherTemperature[20];
int main(void)
{	
	Delay_init();
	DHT11_init();
	MQ_2_Init();
	OLED_Init();
	u8g2_init();

	USART1_init(115200);		//ESP8266
	USART2_init(9600);		//JDY-31
	
	DHT11_ReadData(&Temperature, &Humidity);
	Smoke = MQ_2_Read();
	U8g2_Update(&U8g2);
	ESP8266_ConnectWiFi();
    
	Delay_ms(2000);
	
	ESP8266_Init();
	Delay_ms(1000);
	strcpy(City, "guilin");
	GetWeather(City);
	while (1)
	{
		DHT11_ReadData(&Temperature, &Humidity);
		Smoke = MQ_2_Read();
		Delay_ms(1000);
		U8g2_Update(&U8g2);
		Delay_ms(5000);
		if(USART2_RxFlag) {
			strcpy(City, USART2_Packet);
			WeatherFlag = 0;
			GetWeather(City);
			memset(USART2_Packet, '\0', sizeof(USART2_Packet));
			USART2_RxFlag = 0;
		}
	}
}


uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
	switch(msg) 
	{
		//Initialize I2C peripheral
		case U8X8_MSG_GPIO_AND_DELAY_INIT:
			OLED_I2C_Init();
			Delay_init();
		break;

		//Function which implements a delay, arg_int contains the amount of ms
		case U8X8_MSG_DELAY_MILLI:
			Delay_ms(arg_int);
		break;
		
		//Function which delays 10us
		case U8X8_MSG_DELAY_10MICRO:
			Delay_us(10);
		break;
		
		//Function which delays 100ns
		case U8X8_MSG_DELAY_100NANO:
			__NOP();
		break;
		
		case U8X8_MSG_GPIO_I2C_CLOCK:
			OLED_SCL(arg_int);
		break;
		
		case U8X8_MSG_GPIO_I2C_DATA:
			OLED_SDA(arg_int);
		break;
		
		default:
			return 0; //A message was received which is not implemented, return 0 to indicate an error
	}

	return 1; // command processed successfully.
}

void u8g2_init(void)
{
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&U8g2, U8G2_R0, u8x8_byte_sw_i2c, u8g2_gpio_and_delay_stm32);
	u8g2_InitDisplay(&U8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&U8g2, 0); // wake up display
	u8g2_ClearBuffer(&U8g2);
	Delay_ms(1000);
}

void U8g2_Update(u8g2_t *u8g2)
{
	char str[100];
	u8g2_ClearBuffer(u8g2);
	Delay_ms(200);

	u8g2_DrawLine(u8g2, 0, 0, 127, 0);
	u8g2_DrawLine(u8g2, 0, 1, 127, 1);
	u8g2_DrawLine(u8g2, 0, 33, 127, 33);
	u8g2_DrawLine(u8g2, 0, 34, 127, 34);
	u8g2_DrawLine(u8g2, 0, 35, 127, 35);
	u8g2_DrawLine(u8g2, 0, 48, 127, 48);
	u8g2_DrawLine(u8g2, 0, 49, 127, 49);
	u8g2_DrawLine(u8g2, 0, 50, 127, 50);
	u8g2_DrawLine(u8g2, 0, 62, 127, 62);
	u8g2_DrawLine(u8g2, 0, 63, 127, 63);

	u8g2_DrawLine(u8g2, 0, 0, 0, 63);
	u8g2_DrawLine(u8g2, 1, 0, 1, 63);
	u8g2_DrawLine(u8g2, 2, 0, 2, 63);
	u8g2_DrawLine(u8g2, 125, 0, 125, 63);
	u8g2_DrawLine(u8g2, 126, 0, 126, 63);
	u8g2_DrawLine(u8g2, 127, 0, 127, 63);

	u8g2_DrawLine(u8g2, 63, 33, 63, 63);
	u8g2_DrawLine(u8g2, 64, 33, 64, 63);
	u8g2_DrawLine(u8g2, 65, 33, 65, 63);
	
	u8g2_DrawTriangle(u8g2, 3, 2, 7, 2, 3, 6);
	u8g2_DrawTriangle(u8g2, 125, 2, 121, 2, 125, 6);

	u8g2_DrawTriangle(u8g2, 3, 36, 7, 36, 3, 40);
	u8g2_DrawTriangle(u8g2, 125, 36, 121, 36, 125, 40);
	u8g2_DrawTriangle(u8g2, 3, 28, 3, 33, 7, 33);
	u8g2_DrawTriangle(u8g2, 125, 28, 125, 33, 121, 33);

	u8g2_DrawTriangle(u8g2, 3, 51, 7, 51, 3, 55);
	u8g2_DrawTriangle(u8g2, 125, 51, 121, 51, 125, 55);
	u8g2_DrawTriangle(u8g2, 3, 43, 3, 48, 7, 48);
	u8g2_DrawTriangle(u8g2, 125, 43, 125, 48, 121, 48);

	u8g2_DrawTriangle(u8g2, 3, 57, 3, 62, 7, 62);
	u8g2_DrawTriangle(u8g2, 125, 57, 125, 62, 121, 62);

	u8g2_DrawTriangle(u8g2, 66, 36, 70, 36, 66, 40);
	u8g2_DrawTriangle(u8g2, 63, 36, 59, 36, 63, 40);
	u8g2_DrawTriangle(u8g2, 66, 51, 70, 51, 66, 55);
	u8g2_DrawTriangle(u8g2, 63, 51, 59, 51, 63, 55);
	u8g2_DrawTriangle(u8g2, 66, 43, 66, 48, 70, 48);
	u8g2_DrawTriangle(u8g2, 63, 43, 63, 48, 59, 48);
	u8g2_DrawTriangle(u8g2, 66, 57, 66, 62, 70, 62);
	u8g2_DrawTriangle(u8g2, 63, 57, 63, 62, 59, 62);

	u8g2_DrawLine(u8g2, 3, 33, 3, 63);
	u8g2_DrawLine(u8g2, 62, 33, 62, 63);
	u8g2_DrawLine(u8g2, 66, 33, 66, 63);
	u8g2_DrawLine(u8g2, 124, 33, 124, 63);

	u8g2_SetFont(u8g2, u8g2_font_6x10_mf);
	sprintf(str, "Temp:%d", Temperature);
	u8g2_DrawStr(u8g2, 11, 45, str);
	u8g2_DrawGlyph(u8g2, 54, 45, 0x00B0);
	sprintf(str, "Humi:%d%c", Humidity, '%');
	u8g2_DrawStr(u8g2, 11, 60, str);
	sprintf(str, "Smoke:%d%c", Smoke, '%');
	u8g2_DrawStr(u8g2, 69, 60, str);
	u8g2_SetFont(u8g2, u8g2_font_4x6_mf);
	u8g2_DrawStr(u8g2, 70, 45, City);
	
	if (WeatherFlag) {
		u8g2_SetFont(u8g2, u8g2_font_spleen12x24_mf);
		u8g2_DrawStr(u8g2, 12, 28, WeatherTemperature);

		u8g2_SetFont(u8g2, u8g2_font_6x10_mf);
		u8g2_DrawStr(u8g2, 38, 28, WeatherText);

		u8g2_SetFont(u8g2, u8g2_font_open_iconic_weather_2x_t);
		if(strcmp(WeatherText, "Sunny") == 0)		u8g2_DrawGlyph(u8g2, 100, 28, 0x45);
		else if(strcmp(WeatherText, "Clear") == 0)	u8g2_DrawGlyph(u8g2, 100, 28, 0x42);
		else if((strcmp(WeatherText, "Light Rain") == 0) || (strcmp(WeatherText, "Moderate Rain") == 0) || (strcmp(WeatherText, "Heavy Rain") == 0) || 
		(strcmp(WeatherText, "Storm") == 0) || (strcmp(WeatherText, "Heavy Storm") == 0) || (strcmp(WeatherText, "Severe Storm") == 0))
													u8g2_DrawGlyph(u8g2, 100, 28, 0x43);
		else 		     								u8g2_DrawGlyph(u8g2, 100, 28, 0x40);
	}
	else {
		u8g2_SetFont(u8g2, u8g2_font_6x10_mf);
		u8g2_DrawStr(u8g2, 10, 28, "Getting Weather...");
	}

	u8g2_SendBuffer(u8g2);
	Delay_ms(1000);
}