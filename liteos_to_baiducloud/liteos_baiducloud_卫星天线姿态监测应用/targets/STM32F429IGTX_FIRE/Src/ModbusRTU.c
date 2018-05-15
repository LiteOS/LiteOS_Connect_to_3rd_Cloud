/*
 * Copyright (C) 2018-2019 刘洪峰@叶帆科技   微信：yefanqiu
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ModbusRTU.h"
#include "los_task.h"

//****************************************************************
//RS485接口需要特殊处理一下  如果用RS485则赋值位1，如果不用则用0
//****************************************************************
#define MODBUSRTU_RS485  1  

#define MODBUSRTU_UART_BUFF_SIZE             512
uint8_t MODBUSRTU_UART_BUFF[MODBUSRTU_UART_BUFF_SIZE];
volatile uint16_t MODBUSRTU_UART_BytesToRead = 0;
void MODBUSRTU_USART_IRQHandler(void);
//=========================================================================================
// 串口
//=========================================================================================
//串口初始化
void MODBUSRTU_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd( MODBUSRTU_USART_RX_GPIO_CLK|MODBUSRTU_USART_TX_GPIO_CLK, ENABLE);

  /* Enable UART clock */
  MODBUSRTU_USART_CLKCMD(MODBUSRTU_USART_CLK, ENABLE);
  
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(MODBUSRTU_USART_RX_GPIO_PORT,MODBUSRTU_USART_RX_SOURCE, MODBUSRTU_USART_RX_AF);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(MODBUSRTU_USART_TX_GPIO_PORT,MODBUSRTU_USART_TX_SOURCE,MODBUSRTU_USART_TX_AF);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = MODBUSRTU_USART_TX_PIN  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MODBUSRTU_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Pin = MODBUSRTU_USART_RX_PIN;
  GPIO_Init(MODBUSRTU_USART_RX_GPIO_PORT, &GPIO_InitStructure);
			
  /* USART1 mode config */
  USART_InitStructure.USART_BaudRate = MODBUSRTU_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(MODBUSRTU_USART, &USART_InitStructure); 
  
  USART_Cmd(MODBUSRTU_USART, ENABLE);	
	USART_ClearFlag(MODBUSRTU_USART, USART_FLAG_TC);  
  
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = MODBUSRTU_USART_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
	//注册中断
	LOS_HwiCreate(MODBUSRTU_USART_IRQ, 0,0,MODBUSRTU_USART_IRQHandler,0);
	
  /* 使能串口2接收中断 */
	USART_ITConfig(MODBUSRTU_USART, USART_IT_RXNE, ENABLE);
}

//中断缓存串口数据
void MODBUSRTU_USART_IRQHandler(void)
{
  if(MODBUSRTU_UART_BytesToRead<MODBUSRTU_UART_BUFF_SIZE)
  {
    if(USART_GetITStatus(MODBUSRTU_USART, USART_IT_RXNE) != RESET)
    {			 
        MODBUSRTU_UART_BUFF[MODBUSRTU_UART_BytesToRead++] = (uint8_t)USART_ReceiveData(MODBUSRTU_USART);			
			  //printf("%d=%x\r\n",MODBUSRTU_UART_BytesToRead,MODBUSRTU_UART_BUFF[MODBUSRTU_UART_BytesToRead-1]);			 
    }
  }
}

//清空数据
void MODBUSRTU_UART_DiscardInBuffer(void)
{
  MODBUSRTU_UART_BytesToRead = 0;
	for(int i=0;i<MODBUSRTU_UART_BUFF_SIZE;i++)
	{
    MODBUSRTU_UART_BUFF[i]=0;
  }
}

void MODBUSRTU_UART_Write(uint8_t *DataArray,uint16_t size)
{
#if MODBUSRTU_RS485
	//RS485发送模式
	RS485_GPIO_ON;	
#endif
	
	for(int i=0;i<size;i++)
	{
		 //printf("[s]%d=%x\r\n",i,DataArray[i]);
		 USART_SendData(MODBUSRTU_USART,DataArray[i]);
	 	 while( USART_GetFlagStatus(MODBUSRTU_USART, USART_FLAG_TXE) == RESET );
	}	
	
#if MODBUSRTU_RS485
	//多发一个数据
	USART_SendData(MODBUSRTU_USART,0);
	while( USART_GetFlagStatus(MODBUSRTU_USART, USART_FLAG_TXE) == RESET);
	//RS485接收模式
	RS485_GPIO_OFF;	
#endif
}


//=========================================================================================
// Modbus
//=========================================================================================
//CRC16校验
uint16_t GetCheckCode(uint8_t * buf,int nEnd)
{
	uint16_t crc=(uint16_t)0xffff;
	int i,j;
	for(i = 0; i < nEnd; i++)
	{
		crc^=(uint16_t)buf[i];
		for(j = 0; j < 8; j++)
		{
			if(crc&1)
			{
				crc>>=1;
				crc^=0xA001;
			}
			else
				crc>>=1;
		}
	}
	return crc;
}

//发送数据
int SendCommand(unsigned int intSendNum,unsigned char *byrSendData,unsigned int intInceptNum,uint8_t *bytInceptData)
{
	   //清空接收和发送缓冲区
	   MODBUSRTU_UART_DiscardInBuffer();	
	   //发送数据
     MODBUSRTU_UART_Write(byrSendData,intSendNum);	
	   /*
	   for (int t = 0; t < intInceptNum * 2 + 30; t++)
     {
         if (MODBUSRTU_UART_BytesToRead >= intInceptNum) break;
         LOS_TaskDelay(10);		
     }*/	   
		 
     //-----------------------------
		 LOS_TaskDelay(200);		
	   
		 if (MODBUSRTU_UART_BytesToRead >= intInceptNum)
		 { 
 #if MODBUSRTU_RS485
			  memcpy(bytInceptData,&MODBUSRTU_UART_BUFF[1], intInceptNum);
 #else
		  	memcpy(bytInceptData,MODBUSRTU_UART_BUFF, intInceptNum);
 #endif		

        //for(int i=0;i<intInceptNum;i++) printf("%X ",bytInceptData[i]);
			  //printf("\r\n");
			 
			  MODBUSRTU_UART_DiscardInBuffer();	
				return 0;               //数据接收成功
		 }
		 else
		 {
		 		return -1;              //数据接收失败
		 }
}

//数据收发
int RtuData(uint8_t Addr, uint8_t Mode, uint16_t DataStart, uint8_t *DataArray,uint16_t DataNum)
{
    uint8_t bytSendArray[255];          
    uint8_t bytReceiveArray[255];          
	  uint16_t intCRC16;
    int i;
    int intOffSet;
    int intSendNum;
    int intGetDataLen;                  

    if (DataNum>64 || DataNum<1) return 3;
  
    bytSendArray[0] = Addr;                       //设备地址
    bytSendArray[1] = Mode;                       //功能模式
    bytSendArray[2] = DataStart / 256;            //地址高位
    bytSendArray[3] = DataStart & 0xFF;           //地址低位

    bytSendArray[4] = DataNum / 256;              //数据个数高位
    bytSendArray[5] = DataNum & 0xFF;             //数据个数低位

    if (Mode==MODBUSRTU_WriteData)
    {
	     bytSendArray[6] = DataNum * 2;              //数据的字节个数
	     for(i = 1;i<DataNum * 2+1;i++)
	     bytSendArray[6+i] = DataArray[i-1];
	     intOffSet = 7 + DataNum * 2;
    }
    else
    {
	      intOffSet = 6;
    }
    intCRC16=GetCheckCode(bytSendArray,intOffSet);
    bytSendArray[intOffSet] = intCRC16 & 0xFF;                    //CRC校验低位
    bytSendArray[intOffSet + 1] = (intCRC16>>8) &0xff;            //CRC校验高位
    
    intSendNum=intOffSet+2;
    if (Mode==MODBUSRTU_WriteData)  intGetDataLen = 8;
    else   intGetDataLen = 5 + DataNum * 2;
    
    //发送和接收数据
    if (SendCommand(intSendNum,bytSendArray,intGetDataLen,bytReceiveArray)!=0)
    {
	      return 1;
    }

    //信息处理
    intCRC16=GetCheckCode(bytReceiveArray,intGetDataLen-2);

    //CRC16校验
    if (bytReceiveArray[intGetDataLen - 2]==(intCRC16 & 0xFF) && bytReceiveArray[intGetDataLen - 1]==((intCRC16>>8) &0xff))
    {
				//帧数据是否正确
				if (bytReceiveArray[0] == bytSendArray[0] && bytReceiveArray[1] == bytSendArray[1])
				{
					 if( Mode==MODBUSRTU_WriteData)
					 {
							 //
					 }
					 else
					 {
							 for(i=0;i<bytReceiveArray[2];i++)
							 DataArray[i] = bytReceiveArray[3 + i];
					 }
					 return 0;
				}
    }
   return 2;
}

//=========================================================================================================//
