#include "stm32f10x.h"
#include "esp8266.h"
#include "Serial.h"
#include "OLED.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PrivateKey "SmvQn1fWRHY9rAo1p"
char WeatherText[20], WeatherTemperature[20];
extern uint8_t WeatherFlag;
extern char Message[100];
extern char USART1_RX_BUFF[USART1_BUFF_SIZE];

void ExtractJSON(const char* json, const char* key, char* result) {
    const char* keyPos = strstr(json, key);
    if (!keyPos) return;
    keyPos = strchr(keyPos, ':');
    if (!keyPos) return; 
    keyPos++;
    while (*keyPos == ' ' || *keyPos == '"')  keyPos++;
    uint8_t i = 0;
    while (*keyPos != ',' && *keyPos != '"' && *keyPos != '}' && *keyPos != '\0') {
        result[i++] = *keyPos++;
    }
    result[i] = '\0';
}

void GetWeather(char *City) {
    char temp[200];
    uint8_t trytimes = 5;
    sprintf(temp, "GET https://api.seniverse.com/v3/weather/now.json?key=%s&location=%s&language=en", PrivateKey, City);

    strcpy(Message, "TRYING...");

    while (trytimes--) if (!ESP8266_SendCommand(temp, "text", 2000)) break;
    if(trytimes) {
        ExtractJSON(USART1_RX_BUFF, "\"text\"", WeatherText);
        ExtractJSON(USART1_RX_BUFF, "\"temperature\"", WeatherTemperature);
        WeatherFlag = 1;
        strcpy(Message, WeatherText);
    }
    else {
        strcpy(Message, "ERROR");
    }
}