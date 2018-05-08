/*
 * Copyright (C) 2018-2019 刘洪峰@叶帆科技   微信：yefanqiu
 */
/* Includes LiteOS------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "los_base.h"
#include "los_config.h"
#include "los_typedef.h"
#include "los_hwi.h"
#include "los_task.ph"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
//--//
#include "cmsis_os.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "bsp_led.h" 
#include "bsp_debug_usart.h"
#include "dwt.h"
#include "bsp_key.h"
#include "includes.h"
#include "lwipopts/netconf.h"
//--//
#include "iot_import.h"
#include "iot_export.h"
#include "los_mux.h"
//--//
#include "ModbusRTU.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle;

//凌霄综合采集器
__IO INT32  DeviceState = 0;      //设备状态 0-正在初始化网络  1-正在连接服务器 2-正在上传数据  3-收到下行数据  4-故障状态

#define PRODUCT_KEY             "a1koqo3bO0j"
#define DEVICE_NAME             "YFLX01"
#define DEVICE_SECRET           "wI44FSLOJfnVrbcGl4P4sYR8Ko6R2iZ1"

#define ALINK_BODY_FORMAT         "{\"id\":\"%d\",\"version\":\"1.0\",\"params\":%s,\"method\":\"%s\"}"
#define ALINK_TOPIC_PROP_POST     "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"
#define ALINK_TOPIC_PROP_POSTRSP  "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post_reply"
#define ALINK_TOPIC_PROP_SET      "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#define ALINK_METHOD_PROP_POST    "thing.event.property.post"

#define MSG_LEN_MAX             (1024)
//--//
UINT32 SendCount = 0;
//上行属性
volatile static INT32 g_CommState=0;       //设备状态  0-设备不在线 1-设备在线
volatile static float g_T1 = 18.1;         //温度1
volatile static float g_T2 = 18.2;         //温度2
volatile static float g_T3 = 18.3;         //温度3
volatile static float g_Tin = 18;          //内部温度
volatile static float g_H = 35;            //湿度
volatile static float g_Lux = 120;         //光照
volatile static INT32 g_CO2 = 860;         //二氧化碳
volatile static float g_O2 = 21.0;         //氧气
volatile static INT32 g_NH3 = 8;           //氨气
volatile static INT32 g_PM25 = 26;         //PM2.5
//下行属性
volatile static INT32 g_LightSwitch = 0;   //光照开关
volatile static INT32 g_SpraySwitch = 0;   //喷淋开关
volatile static INT32 old_LightSwitch = -1;   //光照开关
volatile static INT32 old_SpraySwitch = -1;   //喷淋开关
void *pclient;
iotx_conn_info_pt pconn_info;
iotx_mqtt_param_t mqtt_params;
iotx_mqtt_topic_info_t topic_msg;
char msg_pub[512];
char *msg_buf = NULL, *msg_readbuf = NULL;
char param[256];

/* Private function prototypes -----------------------------------------------*/
void hardware_init(void)
{
	LED_GPIO_Config();
	Key1_GPIO_Config();
	Key2_GPIO_Config();
	KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
	Debug_USART_Config();
	DelayInit(SystemCoreClock);
	//串口配置
	MODBUSRTU_UART_Init(); 	
	//以太网配置
	ETH_BSP_Config();
}

//----------------------------//
//设备状态
void DeviceStateTask(void)
{
	  int ret = 0;
   	uint8_t Buffer[20];
		uint16_t value[10];
	
	  while(1)
		{
			   //读取设备数据                    
			   ret =RtuData(1, MODBUSRTU_ReadData,0,Buffer,10); 
			   if(ret == 0)
				 {
					   g_CommState=1;
					   for(int i=0;i<10;i++)
					   {
						    value[i] = Buffer[i*2]<<8|Buffer[i*2+1];
					   }					 
					   g_Lux =  value[0];
						 g_H = (float)(value[1]/100.0);
					   g_CO2 =  value[2];
						 g_NH3 =  value[3];
						 g_PM25 =  value[4];
						 g_O2 = (float)(value[5]/100.0);
						 g_T1= (float)(value[6]/100.0);
						 g_T2= (float)(value[7]/100.0);
						 g_T3= (float)(value[8]/100.0);
						 g_Tin = (float)(value[9]/100.0);
				 }
				 else
				 {
					   g_CommState = 0;
					   printf("Read Device Error!!!\r\n");
				 }
				 
				 if(g_LightSwitch!=old_LightSwitch)
				 {
					  uint8_t Buffer[2]={0,(uint8_t)g_LightSwitch};
					  ret = RtuData(1, MODBUSRTU_WriteData,20,Buffer,2); 
					  printf("[LightSwitch Write]ret=%d\r\n",ret);						
						/*if(ret==0)*/ old_LightSwitch = g_LightSwitch;
				 }
				 
				 if(g_SpraySwitch!=old_SpraySwitch)
				 {
					   uint8_t Buffer[2]={0,(uint8_t)g_SpraySwitch};
					   ret = RtuData(1, MODBUSRTU_WriteData,21,Buffer,2); 
						 printf("[SpraySwitch Write]ret=%d\r\n",ret);			
						 /*if(ret==0)*/ old_SpraySwitch = g_SpraySwitch;
				 }			  
			   
			   //显示设备状态
			   if(DeviceState == 0)
				 {					   
					   LED_YELLOW;  //正在初始化网络
				 }
				 else if(DeviceState == 1)
				 {
					   LED_PURPLE;   //正在连接服务器
				 }
				 else if(DeviceState == 2)
				 {
					   LED_GREEN;    //正在上传数据
				 }
				 else if(DeviceState == 3)
				 {
					   LED_BLUE;     //收到下行数据
				 }
				 else if(DeviceState == 4)
				 {
					   LED_RED;      //故障状态
				 }

         if(DeviceState>1 && DeviceState<4)
				 {
					   //handle the MQTT packet received from TCP or SSL connection
					   //处理接收的数据
             //IOT_MQTT_Yield(pclient, 200);		
             LOS_TaskDelay(300);							 
				 }	
				 else
				 {
						 LOS_TaskDelay(500);		
				 }	
				 
			   LED_RGBOFF;
				 LOS_TaskDelay(500);		
		}		
}

//订阅反馈
static void mqtt_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    // print topic name and topic message
    printf("----\r\n");
    printf("Topic: '%.*s' (Length: %d)\r\n",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    printf("Payload: '%.*s' (Length: %d)\r\n",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    printf("----\r\n");
	
	  //下行数据
	  if(ptopic_info->topic_len==50)
		{
			  DeviceState = 3;
			  //光照开关
			  char *ptr = strstr(ptopic_info->payload, "LightSwitch");
			  if(ptr!=NULL)
				{
					  ptr = (char *)&ptr[strlen("LightSwitch")+2];
					  int len = strcspn(ptr,"}");
					  ptr[len]='\0';					
					  g_LightSwitch = atoi(ptr);	
					  printf("LightSwitch=%d\r\n",g_LightSwitch);
				}
				else
				{
					//喷淋开关
					ptr = strstr(ptopic_info->payload, "SpraySwitch");
					if(ptr!=NULL)
					{
							ptr = (char *)&ptr[strlen("SpraySwitch")+2];
							int len = strcspn(ptr,"}");
							ptr[len]='\0';					
							g_SpraySwitch = atoi(ptr);		
						  printf("SpraySwitch=%d\r\n",g_SpraySwitch);
				 }
			 }				
		}
}
//MQTT状态
void mqtt_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            printf("undefined event occur.\r\n");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            printf("MQTT disconnect.\r\n");
				    //连接断开，通知对应程序重连
				    DeviceState = 1;
            break;
        case IOTX_MQTT_EVENT_RECONNECT:
            printf("MQTT reconnect.\r\n");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            printf("subscribe success, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            printf("subscribe wait ack timeout, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            printf("subscribe nack, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            printf("unsubscribe success, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            printf("unsubscribe timeout, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            printf("unsubscribe nack, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            printf("publish success, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            printf("publish timeout, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            printf("publish nack, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            printf("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s\r\n",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;
        
        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            printf("buffer overflow, %s\r\n", (char *)msg->msg);
            break;

        default:
            printf("Should NOT arrive here.\r\n");
            break;
    }
}

//----------------------------------//
void MQTTAlinkTask(void)
{ 
	  int rc = 0;	 
	  if (NULL == (msg_buf = (char *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR,MSG_LEN_MAX))) {
        printf("not enough memory\r\n");
        goto do_exit;
    }
    if (NULL == (msg_readbuf = (char *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR,MSG_LEN_MAX))) {
        printf("not enough memory\r\n");
       goto do_exit;
    }
	  
	  /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        printf("AUTH request failed!\r\n");
        goto do_exit;
    }
		
		/* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));
    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = NULL; //pconn_info->pub_key;
    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;
    mqtt_params.handle_event.h_fp = mqtt_event_handle;
    mqtt_params.handle_event.pcontext = NULL;
		
	  printf("host=%s port=%d\r\n",mqtt_params.host,mqtt_params.port);
		printf("username=%s password=%s\r\n",mqtt_params.username,mqtt_params.password);
		
		//--//
		while(1)
		{			
      //MQTT链接			
			if(DeviceState==1)
			{
			   LOS_TaskDelay(1000);		
         if(pclient!=NULL)
				 {
					   printf("MQTT_Destroy!!!\r\n");
					   IOT_MQTT_Unsubscribe(pclient, ALINK_TOPIC_PROP_POSTRSP);
						 LOS_TaskDelay(200);
						 IOT_MQTT_Destroy(&pclient);		
				 }					 
				 pclient = IOT_MQTT_Construct(&mqtt_params); 
				 if (NULL == pclient)
				 {
						printf("MQTT construct failed\r\n");
				 }
				 else
				 {
					  printf("MQTT construct succeed\r\n");
					
					  //订阅（数据上行反馈）
						rc = IOT_MQTT_Subscribe(pclient, ALINK_TOPIC_PROP_POSTRSP, IOTX_MQTT_QOS1, mqtt_message_arrive, NULL);
						if (rc < 0) {
								IOT_MQTT_Unsubscribe(pclient, ALINK_TOPIC_PROP_POSTRSP);
							  LOS_TaskDelay(200);
								IOT_MQTT_Destroy(&pclient);		
                pclient = NULL;							
								printf("IOT_MQTT_Subscribe() failed, rc = %d\r\n", rc);
							  continue;
						}
						LOS_TaskDelay(1000);	
						
						//订阅（数据下行）
						rc = IOT_MQTT_Subscribe(pclient, ALINK_TOPIC_PROP_SET, IOTX_MQTT_QOS1, mqtt_message_arrive, NULL);
						if (rc < 0) {
							  IOT_MQTT_Unsubscribe(pclient, ALINK_TOPIC_PROP_POSTRSP);
							  LOS_TaskDelay(200);
								IOT_MQTT_Destroy(&pclient);			
                pclient = NULL;												
								printf("IOT_MQTT_Subscribe() failed, rc = %d\r\n", rc);
								continue;
						}		
						LOS_TaskDelay(1000);		
						DeviceState = 2;		
				 }				 
			}
			//数据发送
			else if(DeviceState == 2 || DeviceState==3)
			{			
			  //Initialize topic information
        memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
        topic_msg.qos = IOTX_MQTT_QOS1;
        topic_msg.retain = 0;
        topic_msg.dup = 0;
			
        //read sensor data
        memset(param, 0, sizeof(param));
        memset(msg_pub, 0, sizeof(msg_pub));	
				sprintf(param,  "{\"T1\":%3.1f,\"T2\":%3.1f,\"T3\":%3.1f,\"Tin\":%3.1f,\"H\":%3.1f,\"Lux\":%0.0f,\"CO2\":%d,\"O2\":%3.1f,\"NH3\":%d,\"PM25\":%d,\"LightSwitch\":%d,\"SpraySwitch\":%d,\"CommState\":%d}",
				g_T1,g_T2,g_T3,g_Tin,g_H,g_Lux,g_CO2,g_O2,g_NH3,g_PM25,g_LightSwitch,g_SpraySwitch,g_CommState);
         				
				int msg_len = sprintf(msg_pub, ALINK_BODY_FORMAT, ++SendCount, param,ALINK_METHOD_PROP_POST);
			  if (msg_len < 0) {
            printf("Error occur! Exit program\r\n");
					  break;
        }
								
        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;        
        rc = IOT_MQTT_Publish(pclient, ALINK_TOPIC_PROP_POST, &topic_msg);
        if (rc < 0) {
            printf("error occur when publish. %d\r\n", rc);
				  	IOT_MQTT_Unsubscribe(pclient, ALINK_TOPIC_PROP_POSTRSP);
					  LOS_TaskDelay(200);
					  IOT_MQTT_Destroy(&pclient);
					  pclient = NULL;					
					  DeviceState = 1;   //重新连接
					  continue;
        }						
        printf("packet-id=%u, publish topic msg=%s\r\n", (uint32_t)rc, msg_pub);
				IOT_MQTT_Yield(pclient, 200);	

				DeviceState = 2;
				LOS_TaskDelay(500);		    //0.5秒上传一次数据		
			}		
      LOS_TaskDelay(20);			
    }   		
		
		//----------------------------------------//
    IOT_MQTT_Unsubscribe(pclient, ALINK_TOPIC_PROP_POSTRSP);
    LOS_TaskDelay(200);
    IOT_MQTT_Destroy(&pclient);
		
do_exit:
	  DeviceState = 4;
	  if (NULL != msg_buf)
    {
        LOS_MemFree((VOID *)OS_SYS_MEM_ADDR,msg_buf);
    }
    if (NULL != msg_readbuf) 
    {
			LOS_MemFree((VOID *)OS_SYS_MEM_ADDR,msg_readbuf);
    }		
}

//-----------------------------------------------------------------------------
//创建设备任务
UINT32 Create_DeviceStateTask()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "DeviceStateTask";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)DeviceStateTask;
    task_init_param.uwStackSize =0x1000;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;        
}

#ifdef USE_DHCP
UINT32 Create_DHCPTask()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "DHCP";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)LwIP_DHCP_task;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;        
}
#endif

//创建发送数据任务
UINT32 Create_MQTTAlinkTask()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "MQTTAlinkTask";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)MQTTAlinkTask;
    task_init_param.uwStackSize =0x1000;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;        
}

//======================================================================
//主函数
//======================================================================
int main(void)
{
    UINT32 uwRet = LOS_OK;
		
	  //内核初始化	
    LOS_KernelInit();
	  //硬件初始化
    hardware_init();  
	  
	  printf("-------------- LiteOS + Alink ---------------\r\n");  
		//Initilaize the LwIP stack
	  LwIP_Init();
	
		//设备状态+MQTT反馈信息任务
		uwRet = Create_DeviceStateTask();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }

#ifdef USE_DHCP		
    //创建DHCP任务
    printf("Create DHCP task...\r\n");  
    uwRet = Create_DHCPTask();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
#endif
		
		//MQTT+Alink
		uwRet = Create_MQTTAlinkTask();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
		
		//启动LiteOS
    LOS_Start();
}
