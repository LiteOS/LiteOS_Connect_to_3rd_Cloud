#ifndef __MODEL_USART_H
#define __MODEL_USART_H

#include "stm32f4xx.h"
#include <stdio.h>
#include "los_typedef.h"



//引脚定义
/*******************************************************/
#define MODEL_USART                             USART2
#define MODEL_USART_CLK                         RCC_APB1Periph_USART2

#define MODEL_USART_RX_GPIO_PORT                GPIOD
#define MODEL_USART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define MODEL_USART_RX_PIN                      GPIO_Pin_6
#define MODEL_USART_RX_AF                       GPIO_AF_USART2
#define MODEL_USART_RX_SOURCE                   GPIO_PinSource6

#define MODEL_USART_TX_GPIO_PORT                GPIOD
#define MODEL_USART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define MODEL_USART_TX_PIN                      GPIO_Pin_5
#define MODEL_USART_TX_AF                       GPIO_AF_USART2
#define MODEL_USART_TX_SOURCE                   GPIO_PinSource5


#define MODEL_USART_IRQHandler                  USART2_IRQHandler
#define MODEL_USART_IRQ                         USART2_IRQn
/************************************************************/
#define MODEL_RESET_GPIO_PORT                   GPIOH
#define MODEL_RESET_GPIO_CLK                    RCC_AHB1Periph_GPIOH
#define MODEL_RESET_PIN                         GPIO_Pin_5

#define MODEL_RESET(a) if (a)  \
                    GPIO_SetBits(MODEL_RESET_GPIO_PORT,MODEL_RESET_PIN);\
                    else        \
                    GPIO_ResetBits(MODEL_RESET_GPIO_PORT,MODEL_RESET_PIN)


//串口波特率
#define MODEL_USART_BAUDRATE                    9600



void MODEL_USART_Config(void);

void MODEL_RESET_Config(void);

void MODEL_USART_IRQHandler(void);

void MODEL_USART_printf(char *Data,...);

void MODEL_MSG_QueueCreate(void);

UINT32 MODEL_MSG_QueueRead(char *rx_buff, UINT32 TimeOut);


#endif /* __USART1_H */
