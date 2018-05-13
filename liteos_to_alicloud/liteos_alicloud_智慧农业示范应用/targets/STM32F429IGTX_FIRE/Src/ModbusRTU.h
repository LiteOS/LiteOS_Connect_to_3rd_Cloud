#ifndef __MODBUSRTU_USART_H
#define	__MODBUSRTU_USART_H

#include "stm32f4xx.h"

#define MODBUSRTU_USART                             USART2
#define MODBUSRTU_USART_CLK                         RCC_APB1Periph_USART2
#define MODBUSRTU_USART_CLKCMD                      RCC_APB1PeriphClockCmd
#define MODBUSRTU_USART_BAUDRATE                    9600
#define MODBUSRTU_USART_IRQ                         USART2_IRQn
//#define MODBUSRTU_USART_IRQHandler                  USART2_IRQHandler

#define MODBUSRTU_USART_TX_GPIO_PORT                GPIOD
#define MODBUSRTU_USART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define MODBUSRTU_USART_TX_PIN                      GPIO_Pin_5
#define MODBUSRTU_USART_TX_AF                       GPIO_AF_USART2
#define MODBUSRTU_USART_TX_SOURCE                   GPIO_PinSource5

#define MODBUSRTU_USART_RX_GPIO_PORT                GPIOD
#define MODBUSRTU_USART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define MODBUSRTU_USART_RX_PIN                      GPIO_Pin_6
#define MODBUSRTU_USART_RX_AF                       GPIO_AF_USART2
#define MODBUSRTU_USART_RX_SOURCE                   GPIO_PinSource6


//收发管脚
#define RS485_GPIO_PIN             GPIO_Pin_11                 
#define RS485_GPIO_PORT            GPIOD                     
#define RS485_GPIO_CLK             RCC_AHB1Periph_GPIOD

#define	digitalHi(p,i)			{p->BSRRL=i;}			  //设置为高电平		
#define digitalLo(p,i)			{p->BSRRH=i;}				//输出低电平
#define RS485_GPIO_ON	    	digitalHi(RS485_GPIO_PORT,RS485_GPIO_PIN)
#define RS485_GPIO_OFF			digitalLo(RS485_GPIO_PORT,RS485_GPIO_PIN)

#define MODBUSRTU_ReadData    3              //读
#define MODBUSRTU_WriteData   16             //写

void MODBUSRTU_UART_Init(void);
void MODBUSRTU_UART_DiscardInBuffer(void);
void MODBUSRTU_UART_Write(uint8_t *DataArray,uint16_t size);
int RtuData(uint8_t Addr, uint8_t Mode, uint16_t DataStart, uint8_t *DataArray,uint16_t DataNum);
#endif /* __MODBUSRTU_USART_H */
