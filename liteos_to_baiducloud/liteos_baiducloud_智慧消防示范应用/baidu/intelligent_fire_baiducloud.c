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
#include "intelligent_fire_baiducloud.h"
#include "los_task.ph"
#include "bsp_debug_usart.h"
#include "netconf.h"
#include "tcp_echoclient.h"
#include "stm32f429_phy.h"
#include "bsp_led.h" 
#include "bsp_key.h"

#include "iothub_mqtt_client.h"

extern KEY Key1,Key2;
extern __IO uint32_t LocalTime;
UINT32 g_KeyTskHandle;

VOID LwipAppInit(void)
{
	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();	
	PRINTF_DBG("PHY初始化结束\n");

	/* Initilaize the LwIP stack */
	LwIP_Init();	

	PRINTF_DBG("    KEY1: 启动/断开TCP连接\n");
//	PRINTF_DBG("    KEY2: 断开TCP连接\n");

	/* IP地址和端口可在netconf.h文件修改，或者使用DHCP服务自动获取IP
	(需要路由器支持)*/
	PRINTF_DBG("本地IP和端口: %d.%d.%d.%d\n",IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
	PRINTF_DBG("远端IP和端口: %d.%d.%d.%d:%d\n",DEST_IP_ADDR0, DEST_IP_ADDR1,DEST_IP_ADDR2, DEST_IP_ADDR3,DEST_PORT);
}

unsigned char mqtt_server_connstatus = 0;
unsigned char mqtt_send_alarm_flg = 0;
VOID key_task(VOID)
{
	while(1)
	{
		Key_RefreshState(&Key1);//刷新按键状态
		Key_RefreshState(&Key2);//刷新按键状态
		if(Key_AccessTimes(&Key1,KEY_ACCESS_READ)!=0)
		{
			if(mqtt_server_connstatus==0)
			{
				if (EthLinkStatus == 0)
				{
					connect_mqtt_server();
				}
			}
			else
			{
				disconnect_mqtt_server();
			}
			Key_AccessTimes(&Key1,KEY_ACCESS_WRITE_CLEAR);
		}
		if(Key_AccessTimes(&Key2,KEY_ACCESS_READ)!=0)
		{
			if(mqtt_send_alarm_flg)
			{
				PRINTF_DBG("\r\n mqtt client stop send alarm message !!!");
				mqtt_send_alarm_flg = 0;
			}
			else if(mqtt_send_alarm_flg==0)
			{
				PRINTF_DBG("\r\n mqtt client start send alarm message !!!");
				mqtt_send_alarm_flg = 1;
			}
			Key_AccessTimes(&Key2,KEY_ACCESS_WRITE_CLEAR);
		}
	}
}

UINT32 creat_key_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "key_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)key_task;
    task_init_param.uwStackSize = 0x800;

    uwRet = LOS_TaskCreate(&g_KeyTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

UINT32 g_LwipPollTskHandle;

VOID lwip_poll_task(VOID)
{
	LwipAppInit();
	while(1)
	{
		/* check if any packet received */
		if (ETH_CheckFrameReceived())
		{ 
			/* process received ethernet packet */
			LwIP_Pkt_Handle();
		}
		/* handle periodic timers for LwIP */
		LwIP_Periodic_Handle(LocalTime);
	}
}

UINT32 creat_lwip_poll_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "lwip_poll_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)lwip_poll_task;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_LwipPollTskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}






























/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

