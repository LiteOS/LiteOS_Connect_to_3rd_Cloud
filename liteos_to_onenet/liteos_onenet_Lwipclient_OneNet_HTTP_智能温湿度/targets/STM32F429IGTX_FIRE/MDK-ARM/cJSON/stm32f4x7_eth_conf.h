#ifndef __STM32F4x7_ETH_CONF_H
#define __STM32F4x7_ETH_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"

#define USE_ENHANCED_DMA_DESCRIPTORS

//#define USE_Delay

#ifdef USE_Delay
  #include "main.h"                /* Header file where the Delay function prototype is exported */  
  #define _eth_delay_    Delay     /* User can provide more timing precise _eth_delay_ function */
#else
  #define _eth_delay_    ETH_Delay /* Default _eth_delay_ function with less precise timing */
#endif


#ifdef  CUSTOM_DRIVER_BUFFERS_CONFIG
/* Redefinition of the Ethernet driver buffers size and count */   
 #define ETH_RX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for receive */
 #define ETH_TX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for transmit */
 #define ETH_RXBUFNB        20                  /* 20 Rx buffers of size ETH_RX_BUF_SIZE */
 #define ETH_TXBUFNB        5                   /* 5  Tx buffers of size ETH_TX_BUF_SIZE */
#endif


/* PHY配置块 **************************************************/
/* PHY复位延时*/ 
#define PHY_RESET_DELAY    ((uint32_t)0x000FFFFF) 

/* PHY配置延时*/ 
#define PHY_CONFIG_DELAY   ((uint32_t)0x00FFFFFF)

//PHY的状态寄存器,用户需要根据自己的PHY芯片来改变PHY_SR的值
//#define PHY_SR    ((uint16_t)16) //DP83848的PHY状态寄存器
#define PHY_SR		((uint16_t)31) //LAN8720的PHY状态寄存器地址

//速度和双工类型的的值,用户需要根据自己的PHY芯片来设置
//#define PHY_SPEED_STATUS            ((uint16_t)0x0002) /*DP83848 PHY速度值*/
//#define PHY_DUPLEX_STATUS           ((uint16_t)0x0004) /*DP83848 PHY连接状态值*/
#define PHY_SPEED_STATUS            ((uint16_t)0x0004) /*LAN8720 PHY速度值*/
#define PHY_DUPLEX_STATUS           ((uint16_t)0x00010) /*LAN8720 PHY连接状态值*/  

#ifdef __cplusplus
}
#endif

#endif 


