/*********************************************Copyright (c)***********************************************
** Wuhan CCEM
**
**--------------File Info---------------------------------------------------------------------------------
** File name:              
** Latest modified Date:    
** Latest Version:          
** Descriptions:            
**
**--------------------------------------------------------------------------------------------------------
** Created by:              shhuang
** Created date:            2018-05-11
** Version:                 V1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/********************************************************************************************************
** Includes
*********************************************************************************************************/
#include "transport.h"
#include "tcp_echoclient.h"


void transport_open(void)
{
	/*connect to tcp server */ 
	tcp_connet_creat();
}	

void transport_close(void)
{
	tcp_connet_close();
}

void transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
	send_tcp_data(buf,buflen);
}

int transport_getdata(unsigned char* buf, int count)
{
	return receive_tcp_data(buf,count);
}





























/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

