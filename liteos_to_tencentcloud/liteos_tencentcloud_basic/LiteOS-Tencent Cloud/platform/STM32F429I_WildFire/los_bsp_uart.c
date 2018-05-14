#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"

#include "los_bsp_uart.h"

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

///重定向c库函数printf到USART1
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到USART1 */
    USART_SendData(DEBUG_USART, (uint8_t) ch);
    
    /* 等待发送完毕 */
    while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);		
	
    return (ch);
}

void LOS_EvbUartInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    RCC_AHB1PeriphClockCmd( DEBUG_USART_RX_GPIO_CLK|DEBUG_USART_TX_GPIO_CLK, ENABLE);
    
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
    
    /* Connect PXx to USARTx_Tx*/
    GPIO_PinAFConfig(DEBUG_USART_RX_GPIO_PORT,DEBUG_USART_RX_SOURCE, DEBUG_USART_RX_AF);
    
    /* Connect PXx to USARTx_Rx*/
    GPIO_PinAFConfig(DEBUG_USART_TX_GPIO_PORT,DEBUG_USART_TX_SOURCE,DEBUG_USART_TX_AF);
    
    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    
    GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_PIN  ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);
    
    /* Configure USART Rx as alternate function  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_PIN;
    GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
    
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_USART, &USART_InitStructure); 
    USART_Cmd(DEBUG_USART, ENABLE);
	
	USART_ClearFlag(DEBUG_USART, USART_FLAG_TC);
    
    return ;
}


/*************************************************************************************************
 *  功能：向串口1发送一个字符                                                                    *
 *  参数：(1) 需要被发送的字符                                                                   *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void LOS_EvbUartWriteByte(char c)
{
    /* 发送一个字节数据到USART1 */
    USART_SendData(DEBUG_USART, (uint8_t) c);
    
    /* 等待发送完毕 */
    while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);		
    
    return ;
}

/*************************************************************************************************
 *  功能：向串口1发送一个字符串                                                                  *
 *  参数：(1) 需要被发送的字符串                                                                 *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void LOS_EvbUartWriteStr(const char* str)
{
    while (*str)
    {
        LOS_EvbUartWriteByte(*str);
        
        str++;
    }

    return ;
}
