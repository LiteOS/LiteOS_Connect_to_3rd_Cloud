#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"



#define RHEOSTAT_ADC_GPIO_PORT GPIOA
#define RHEOSTAT_ADC_GPIO_PIN GPIO_Pin_3
#define RHEOSTAT_ADC_GPIO_CLK RCC_AHB1Periph_GPIOA

#define RHEOSTAT_ADC ADC1
#define RHEOSTAT_ADC_CLK RCC_APB2Periph_ADC1
#define RHEOSTAT_ADC_CHANNEL ADC_Channel_3



extern uint16_t ADC_ConvertedValue;

void Rheostat_ADC_Mode_Config(void);
void Rheostat_ADC_GPIO_Config(void);
uint16_t Rheostat_ADC_StartConv(void);
void Rheostat_ADC_EndConv(void);
void Rheostat_ADC_Init();

#endif
