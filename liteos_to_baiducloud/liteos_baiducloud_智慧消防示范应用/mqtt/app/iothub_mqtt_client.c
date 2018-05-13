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
#include "iothub_mqtt_client.h"
#include "bsp_debug_usart.h"

#include "tcp_echoclient.h"
#include "MQTTPacket.h"
#include "transport.h"

#include "bsp_led.h" 
#include "los_swtmr.ph"
#include "los_sem.h"

UINT32 g_usGetNetStatusSemID;
UINT32 g_usGetNetDataSemID;
UINT32 g_usWaitSubAckSemID;
UINT32 g_usWaitPingRspSemID;

UINT32 g_RecTopicMsgTskHandle;
UINT32 g_SubTopicTskHandle;
UINT32 g_SndMqttPingTskHandle;
UINT32 g_SndMqttAlarmTskHandle;

extern unsigned char mqtt_server_connstatus;
extern unsigned char mqtt_send_alarm_flg;

UINT32 creat_rec_mqtt_message_task(void);
UINT32 creat_mqtt_client_subtopic_task(void);
UINT32 creat_send_mqtt_ping_taask(void);
UINT32 creat_send_alarm_task(void);



#define MQTT_CLIENT_ID						"tbs_n110_001"
#define KEEPALIVE_INTERVAL 					60
#define MQTT_CLIENT_USER_NAME				"tbs_n110/tbs_n110_001"
#define MQTT_CLIENT_PASSWORD				"nu/005K89vIWmbqmP9+V4h7k2GeofYtvZF3o9JT/X8o="

#define MQTT_CLIENT_SUBTOPIC				"topic02"
#define MQTT_CLIENT_PUBTOPIC				"topic01"

#define BUF_SIZE	200
void transport_open(void);
void transport_close(void);
void transport_sendPacketBuffer(unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);

//connect the  mqtt server
void connect_mqtt_server(void)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char buf[BUF_SIZE];
	int buflen = sizeof(buf);
	int len = 0;
	
	UINT32 uwRet1;
	UINT32 uwRet2;
	
	data.clientID.cstring = MQTT_CLIENT_ID;
	data.keepAliveInterval = KEEPALIVE_INTERVAL;
	data.cleansession = 1;
	data.username.cstring = MQTT_CLIENT_USER_NAME;
	data.password.cstring = MQTT_CLIENT_PASSWORD;
	
	clear_rec_buf();
	LOS_SemCreate(0,&g_usGetNetStatusSemID);
	
	transport_open();
	uwRet1 = LOS_SemPend(g_usGetNetStatusSemID,3000);
	if(uwRet1 == LOS_OK)
	{
		PRINTF_DBG("\r\n connect to tcp server success !!!");
		
		len = MQTTSerialize_connect(buf, buflen, &data);
		LOS_SemCreate(0,&g_usGetNetDataSemID);
		transport_sendPacketBuffer(buf, len);
		uwRet2 = LOS_SemPend(g_usGetNetDataSemID,3000);
		if(uwRet2 == LOS_OK)
		{
			/* wait for connack */
			if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
			{
				unsigned char sessionPresent, connack_rc;

				if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
				{
					PRINTF_DBG("Unable to connect, return code %d\n", connack_rc);
					goto exit;
				}
				LED2_ON;
				mqtt_server_connstatus = 1;
				PRINTF_DBG("\r\n connect to mqtt server success !!!");
				
				clear_rec_buf();
				LOS_SemDelete(g_usGetNetStatusSemID);
				LOS_SemDelete(g_usGetNetDataSemID);
				//receive subscribe topic message
				creat_rec_mqtt_message_task();
				//subscribe the toppic
				creat_mqtt_client_subtopic_task();
				//send ping message to mqtt server
				creat_send_mqtt_ping_taask();
				//send mqtt alarm to mqtt server
				creat_send_alarm_task();

				return ;
			}
			else
			{
				PRINTF_DBG("\r\n no receive mqtt server connack !!!");
				
				goto exit;
			}				
		}
		if(uwRet2 == LOS_ERRNO_SEM_TIMEOUT)
		{
			PRINTF_DBG("\r\n connect to mqtt server timeout !!!");
			
			exit:
				clear_rec_buf();
				LOS_SemDelete(g_usGetNetStatusSemID);
				LOS_SemDelete(g_usGetNetStatusSemID);
				transport_close();
		}
	}
	if(uwRet1 == LOS_ERRNO_SEM_TIMEOUT)
	{
		PRINTF_DBG("\r\n connect to tcp server timeout !!!");
		LOS_SemDelete(g_usGetNetStatusSemID);
	}
}
//disconnect the mqtt server
void disconnect_mqtt_server(void)
{
	unsigned char buf[BUF_SIZE];
	int buflen = sizeof(buf);
	int len = 0;
	LOS_TaskDelete(g_RecTopicMsgTskHandle);
	LOS_TaskDelete(g_SubTopicTskHandle);
	LOS_TaskDelete(g_SndMqttPingTskHandle);
	LOS_TaskDelete(g_SndMqttAlarmTskHandle);

	len = MQTTSerialize_disconnect(buf, buflen);
	transport_sendPacketBuffer(buf, len);
	PRINTF_DBG("\r\n close mqtt server connect !!!");
	clear_rec_buf();
	transport_close();
	PRINTF_DBG("\r\n close tcp server connect !!!");
	LED2_OFF;
	mqtt_server_connstatus = 0;
}
//send ping to mqtt server
void send_mqtt_ping_taask(void)
{
	UINT32 uwRet;
	unsigned char buf[BUF_SIZE];
	int buflen = sizeof(buf);
	int len = 0;
	
	while(1)
	{
		LOS_TaskDelay((KEEPALIVE_INTERVAL)*1000);
		
		PRINTF_DBG("\r\n start send ping to  mqtt !!!");
	
		len = MQTTSerialize_pingreq(buf, buflen);
		
		LOS_SemCreate(0,&g_usWaitPingRspSemID);
		transport_sendPacketBuffer(buf, len);
		uwRet = LOS_SemPend(g_usWaitPingRspSemID,3000);
		LOS_SemDelete(g_usWaitPingRspSemID);
		if(uwRet == LOS_OK)
		{
			PRINTF_DBG("\r\n receive mqtt server ping response success!!!");
		}
		if(uwRet == LOS_ERRNO_SEM_TIMEOUT)
		{
			PRINTF_DBG("\r\n receive mqtt server ping response timeout!!!");
			disconnect_mqtt_server();
		}
	}
}

UINT32 creat_send_mqtt_ping_taask(void)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "send mqtt ping task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)send_mqtt_ping_taask;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_SndMqttPingTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

//subscribe the topic02
void mqtt_client_subtopic_task(void)
{
	unsigned char buf[BUF_SIZE];
	int buflen = sizeof(buf);
	int len = 0;
	int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	int req_qos = 0;
	
	UINT32 uwRet;
	
	/* subscribe */
	topicString.cstring = MQTT_CLIENT_SUBTOPIC;
	
	PRINTF_DBG("\r\n mqtt client start subscribe the topic: %s !!!",topicString.cstring);
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);
	LOS_SemCreate(0,&g_usWaitSubAckSemID);
	transport_sendPacketBuffer(buf, len);
	uwRet = LOS_SemPend(g_usWaitSubAckSemID,3000);
	if(uwRet == LOS_OK)
	{
		PRINTF_DBG("\r\n mqtt client subscribe success !!!");
		LOS_SemDelete(g_usWaitSubAckSemID);

	}
	if(uwRet == LOS_ERRNO_SEM_TIMEOUT)
	{
		PRINTF_DBG("\r\n mqtt client subscribe fail !!!");
		LOS_SemDelete(g_usGetNetDataSemID);
		disconnect_mqtt_server();
	}
	LOS_TaskDelete(g_SubTopicTskHandle);
}

UINT32 creat_mqtt_client_subtopic_task(void)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "mqtt client subtopic task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)mqtt_client_subtopic_task;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_SubTopicTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

//receive subscribe topic message
void rec_subscribed_topic_message(void)
{
	unsigned char buf[BUF_SIZE];
	int buflen = sizeof(buf);
	while(1)
	{
		if(tcp_is_received())
		{
			int RetVal = MQTTPacket_read(buf, buflen, transport_getdata);
			if ( RetVal== PUBLISH)
			{
				unsigned char dup;
				int qos;
				unsigned char retained;
				unsigned short msgid;
				int payloadlen_in;
				unsigned char* payload_in;
				MQTTString receivedTopic;

				MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
						&payload_in, &payloadlen_in, buf, buflen);
				PRINTF_DBG("\r\n topic message arrived from mqtt server\n");
				PRINTF_DBG("\r\n the topic name: %s\n", MQTT_CLIENT_SUBTOPIC);
				PRINTF_DBG("\r\n topic message: %.*s\n", payloadlen_in, payload_in);
				if(strstr((const char *)payload_in,"close the alarm"))
				{
					PRINTF_DBG("\r\n close the alarm");
					mqtt_send_alarm_flg = 0;
				}
			}
			if(RetVal== PINGRESP)
			{
				LOS_SemPost(g_usWaitPingRspSemID);
			}
			if(RetVal== SUBACK)
			{
				unsigned short submsgid;
				int subcount;
				int granted_qos;

				MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
				if (granted_qos != 0)
				{
					PRINTF_DBG("granted qos != 0, %d\n", granted_qos);
					
				}
				else
				{
					LOS_SemPost(g_usWaitSubAckSemID);
				}
			}
			clear_rec_buf();
		}
	}
}

UINT32 creat_rec_mqtt_message_task(void)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "rec_topic_message_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)rec_subscribed_topic_message;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_RecTopicMsgTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

//publish the topic01
void mqtt_client_pubtopic(void)
{
	unsigned char buf[BUF_SIZE];
	int buflen = sizeof(buf);
	int len = 0;
	MQTTString topicString = MQTTString_initializer;
	unsigned char payload[] = "send the alarm";
	topicString.cstring  = MQTT_CLIENT_PUBTOPIC;
	
	len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, payload, sizeof(payload));
	transport_sendPacketBuffer(buf, len);
	
	PRINTF_DBG("\r\n publish the topic message");
}

void send_alarm_task(void)
{
	while(1)
	{
		if(mqtt_send_alarm_flg)
		{
			if(mqtt_server_connstatus == 0)
			{
				PRINTF_DBG("\r\n mqtt server no connect,publish the topic message fail !!!");
			}
			else
			{
				// start send alarm message
				LED4_ON;
				LOS_TaskDelay(1000);
				LED4_OFF;
				mqtt_client_pubtopic();
			}
			LOS_TaskDelay(7000);
		}
	}
}

UINT32 creat_send_alarm_task(void)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "send_alarm_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)send_alarm_task;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_SndMqttAlarmTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}



























/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

