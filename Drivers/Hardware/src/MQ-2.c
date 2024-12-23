#include "stm32f10x.h"
#include "Delay.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include <stdint.h>

#define MQ_2_AO_GPIO_PORT			GPIOA
#define MQ_2_AO_GPIO_PIN			GPIO_Pin_5
#define MQ_2_AO_GPIO_CLK_ENABLE() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
#define MQ_2_AO_ADC_CLK_ENABLE()    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE)

void MQ_2_Init(void)
{
    MQ_2_AO_GPIO_CLK_ENABLE();
    MQ_2_AO_ADC_CLK_ENABLE();
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStruct.GPIO_Pin = MQ_2_AO_GPIO_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MQ_2_AO_GPIO_PORT, &GPIO_InitStruct);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_55Cycles5);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStruct);
    
    ADC_Cmd(ADC1, ENABLE);
    
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

uint16_t MQ_2_Read(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    uint16_t val = ADC_GetConversionValue(ADC1);
    return (uint16_t)((float)val * 100 / 4095);
}