#ifndef __DEBUG_USART_H
#define	__DEBUG_USART_H

#include "stm32f4xx.h"
#include <stdio.h>


//#define USART1_DR_Base  0x40013804		// 0x40013800 + 0x04 = 0x40013804
//#define SENDBUFF_SIZE   5000

#define DEBUG_USART                             USART1
#define DEBUG_USART_CLK                         RCC_APB2Periph_USART1
#define DEBUG_USART_BAUDRATE                    115200

#define DEBUG_USART_RX_GPIO_PORT                GPIOA
#define DEBUG_USART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOA
#define DEBUG_USART_RX_PIN                      GPIO_Pin_10
#define DEBUG_USART_RX_AF                       GPIO_AF_USART1
#define DEBUG_USART_RX_SOURCE                   GPIO_PinSource10

#define DEBUG_USART_TX_GPIO_PORT                GPIOA
#define DEBUG_USART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOA
#define DEBUG_USART_TX_PIN                      GPIO_Pin_9
#define DEBUG_USART_TX_AF                       GPIO_AF_USART1
#define DEBUG_USART_TX_SOURCE                   GPIO_PinSource9

//打印系统信息
#define PRINTF_SYS_OPEN			1
#if PRINTF_SYS_OPEN
#include <stdio.h>
#define PRINTF_SYS(...) printf(__VA_ARGS__)
#else
#define PRINTF_SYS(...)
#endif

//打印错误信息
#define PRINTF_ERR_OPEN			1
#if PRINTF_ERR_OPEN
#include <stdio.h>
#define PRINTF_ERR(...) printf(__VA_ARGS__)
#else
#define PRINTF_ERR(...)
#endif

//打印调试信息
#define PRINTF_DBG_OPEN			1
#if PRINTF_DBG_OPEN
#include <stdio.h>
#define PRINTF_DBG(...) printf(__VA_ARGS__)
#else
#define PRINTF_DBG(...)
#endif

void PrintHexDataBuffers(unsigned char *data,int len);

void Debug_USART_Config(void);
//int fputc(int ch, FILE *f);

#endif /* __USART1_H */
