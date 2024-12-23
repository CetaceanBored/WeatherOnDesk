#include "stm32f10x.h"
#include "Serial.h"
#include "Delay.h"
#include "OLED.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define WiFi_SSID   "5202wifi"
#define WiFi_PWD    "5202sys."
#define IP_ADDRESS  "api.seniverse.com"
#define PORT        80

extern volatile uint16_t USART1_RX_Index;
extern char USART1_RX_BUFF[USART1_BUFF_SIZE];
extern char USART1_TX_BUFF[USART1_BUFF_SIZE];

extern char Message[100];

enum ESP8266_Status_TypeDef {
	WIFI_DISCONNECTED = 0,
	WIFI_CONNECTED = 1,
	Server_CONNECTED = 2
} ESP8266_Status;

/**
 * @brief SP8266_CheckAccept
 * @return 1:Success; 0:Fail
 * @note Add '\0' before matching response
 */
uint8_t* ESP8266_CheckAccept(char *Response)
{
    char *str = 0;
    if (USART1_RX_Index & (1 << 15)) {
        USART1_RX_BUFF[USART1_RX_Index & ~(1 << 15)] = '\0';
        str = strstr((const char*)USART1_RX_BUFF, (const char*)Response);
    }
    return (uint8_t*)str;
}

/** 
 * @brief ESP8266_SendCommand
 * @return 0:Success; 1:Fail
 */
uint8_t ESP8266_SendCommand(char *Command, char *Response, uint16_t WaitTime)
{
    uint8_t res = 1;
    USART1_RX_Index = 0;
    memset(USART1_RX_BUFF, 0, sizeof(USART1_RX_BUFF));
    char temp[200];
    sprintf(temp, "%s\r\n", Command);
    USART1_SendString(temp);
    Delay_ms(10);
    if(WaitTime) {
        while (WaitTime--) {
            Delay_ms(1);
            if(USART1_RX_Index & (1 << 15)) {           
                if(ESP8266_CheckAccept(Response)) {
                    res = 0;
                    strcpy(Message, Command);
                    break;
                }
                USART1_RX_Index = 0;
            }
        }
    }
    return res;
}

char temp[200];
uint8_t trytimes;
/**
 * @brief ESP8266_ConnectWiFi
 * @return 0:Success; 1:Fail
 */
uint8_t ESP8266_ConnectWiFi(void)
{
    trytimes = 5;
    while(trytimes--) if(!ESP8266_SendCommand("AT", "OK", 2000)) break;

    trytimes = 5;
    while(trytimes--) if(ESP8266_SendCommand("AT+CWMODE=1", "OK", 2000)) break;
    Delay_ms(1000);

    trytimes = 5;
    sprintf(temp, "AT+CWJAP=\"%s\",\"%s\"", WiFi_SSID, WiFi_PWD);
    while(trytimes--) if(ESP8266_SendCommand(temp, "WIFI GOT IP", 5000)) break;
    Delay_ms(1000);

    return 0;
}


/**
 * @brief ESP8266_StartTransparentTransmission
 * @return 0:Success; 1:Fail
 */
uint8_t ESP8266_StartTransparentTransmission(void)
{
    trytimes = 5;
    while (--trytimes) if (!ESP8266_SendCommand("AT", "OK", 2000)) break;
    if(trytimes)  strcpy(Message, "AT OK");//OLED_ShowString(6, 0, "AT OK", 6);
    else strcpy(Message, "AT FAIL");//OLED_ShowString(6, 0, "AT FAIL", 6);
    
    trytimes = 5;
    sprintf(temp, "AT+CIPSTART=\"TCP\",\"%s\",%d", IP_ADDRESS, PORT);
    while (--trytimes) if (!ESP8266_SendCommand(temp, "OK", 5000)) break;
    if (trytimes)  strcpy(Message, "Connected");//OLED_ShowString(6, 0, "CIPSTART OK", 6);
    else strcpy(Message, "CIPSTART FAIL");//OLED_ShowString(6, 0, "CIPSTART FAIL", 6);
    Delay_ms(200);
    

    trytimes = 5;
    while (--trytimes) if(!ESP8266_SendCommand("AT+CIPMODE=1", "OK", 200)) break;
    if (trytimes)  strcpy(Message, "CIPMODE OK");//OLED_ShowString(6, 0, "AT+CIPMODE OK", 6);
    else strcpy(Message, "CIPMODE FAIL");//OLED_ShowString(6, 0, "AT+CIPMODE FAIL", 6);
    Delay_ms(200);

    trytimes = 5;
    while (--trytimes) if(!ESP8266_SendCommand("AT+CIPSEND", "OK", 100)) break;
    if (trytimes) strcpy(Message, "CIPSEND OK");//OLED_ShowString(6, 0, "AT+CIPSEND OK", 6);
    else strcpy(Message, "CIPSEND FAIL");       //OLED_ShowString(6, 0, "AT+CIPSEND FAIL", 6);

    Delay_ms(1000);
    return 0;
}



/**
 * @brief ESP8266_QuitTranparentTransmission
 * @return uint8_t 
 */
uint8_t ESP8266_QuitTransparentTransmission(void)
{
    uint8_t res = 1;
    USART1_SendString("+++");
    Delay_ms(1000);
    res = ESP8266_SendCommand("AT", "OK", 200);
    if (!res) {
        while (ESP8266_SendCommand("AT+CIPMODE=0", "OK", 200));
        while (ESP8266_SendCommand("AT+CIPCLOSE", "OK", 200));
    }
    return res;
}

void ESP8266_Init(void)
{
    strcpy(Message, "Welcome");
    
    trytimes = 5;
	while(ESP8266_SendCommand("AT", "OK", 1000) && --trytimes)
    {
        Delay_ms(100);
        strcpy(Message, "Initialize...");
    }
    if (trytimes == 0) {
        ESP8266_QuitTransparentTransmission();
        Delay_ms(1000);
    }
    
    
    
	if(!ESP8266_SendCommand("AT+AWJAP?", "NO AP", 200)) 
	{
		uint8_t trytimes = 3;
        
		while(trytimes--) if(!ESP8266_ConnectWiFi()) break;
		if(!trytimes) {
			ESP8266_Status = WIFI_CONNECTED;
            strcpy(Message, "WiFi CONNECTED");//
		}
		else {
            strcpy(Message, "OH NO");//
		}
	}
	else {
		ESP8266_Status = WIFI_CONNECTED;
	}
    Delay_ms(500);
    
	if (ESP8266_Status == WIFI_CONNECTED) {
        
		if(!ESP8266_StartTransparentTransmission()) {
			ESP8266_Status = Server_CONNECTED;
			//OLED_ShowString(4, 0, "                ", 6);
			//OLED_ShowString(4, 0, "Server CONNECTED", 6);
            strcpy(Message, "Server Connected");//
			Delay_ms(500);
		}
		else {
			//OLED_ShowString(4, 0, "                ", 6);
			//OLED_ShowString(4, 0, "Something Wrong", 6);
            strcpy(Message, "ERROR");//
            Delay_ms(500);
		}
	}
}
