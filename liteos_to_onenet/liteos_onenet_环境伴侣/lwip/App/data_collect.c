#include "data_collect.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "bsp_adc.h"

u8 CollectDataBuf[MAX_BUF_SIZE]={0};
u8 CollectDataLen=0;
u8 CollectDataSendFlag=0;

float Temperature=15.0;
u32 Ligthness=200;

u8 getTempHttpData(void)
{
	u8 temp[10]={0};
	u8 temp1[100]={0};
	
//	if(CollectDataSendFlag==1)
//		return 0;

	strcpy(CollectDataBuf,"POST /devices/30962671/datapoints HTTP/1.1\r\n");
	strcat(CollectDataBuf,"api-key:MxVZE67SCEn0U0MhwvwkBcGbHBQ=\r\n");
	strcat(CollectDataBuf,"Host:api.heclouds.com\r\n");
	
	strcpy(temp1,"{\"datastreams\":[{\"id\":\"temperature\",\"datapoints\":[{\"value\":");
	sprintf(temp,"%d.%d",(unsigned int)Temperature,(unsigned int)(Temperature-(unsigned int)Temperature)*1000);
	//sprintf(temp,"%06.1f",Temperature);
	strcat(temp1,temp);
	strcat(temp1,"}]}]}");
	
	sprintf(temp,"%d",strlen(temp1));
	strcat(CollectDataBuf,"Content-Length:");
	strcat(CollectDataBuf,temp);
	strcat(CollectDataBuf,"\r\n\r\n");
	
	strcat(CollectDataBuf,temp1);
	CollectDataLen=strlen(CollectDataBuf);
	CollectDataSendFlag=1;
	
	return 1;
		
}

u8 getLightData(void)
{
	u8 temp[10]={0};
	u8 temp1[100]={0};
	
//	if(CollectDataSendFlag==1)
//		return 0;
	Ligthness=ADC_ConvertedValue;
	strcpy(CollectDataBuf,"POST /devices/30962671/datapoints HTTP/1.1\r\n");
	strcat(CollectDataBuf,"api-key:MxVZE67SCEn0U0MhwvwkBcGbHBQ=\r\n");
	strcat(CollectDataBuf,"Host:api.heclouds.com\r\n");
	
	strcpy(temp1,"{\"datastreams\":[{\"id\":\"light\",\"datapoints\":[{\"value\":");
	
	sprintf(temp,"%d",Ligthness);
	strcat(temp1,temp);
	strcat(temp1,"}]}]}");
	
	sprintf(temp,"%d",strlen(temp1));
	strcat(CollectDataBuf,"Content-Length:");
	strcat(CollectDataBuf,temp);
	strcat(CollectDataBuf,"\r\n\r\n");
	
	strcat(CollectDataBuf,temp1);
	CollectDataLen=strlen(CollectDataBuf);
	CollectDataSendFlag=1;
	
	return 1;
}
























