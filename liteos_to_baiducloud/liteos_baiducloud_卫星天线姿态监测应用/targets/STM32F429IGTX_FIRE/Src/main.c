/*
 * 基于百度云+MPU6050
 * 卫星天线姿态监测应用
 * MQTT+LWIP代码参考
 * https://github.com/LiteOS/LiteOS_Connect_to_3rd_Cloud/tree/master/liteos_to_alicloud/liteos_alicloud_智慧农业示范应用
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

#include "MQTTPacket.h"
#include "MQTTConnect.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle;



#define PRODUCT_KEY             "liteos"
#define DEVICE_NAME             "imu"
#define DEVICE_SECRET           "WVpwvt1cnfSvJ2yZzvXls+lnnwZf37Vxv8esxU6kXlk="

#define IOT_TOPIC_MPU_POST     "mpu"
#define IOT_TOPIC_MPU_POSTRSP  "mpu"
#define IOT_TOPIC_MPU_SET      "mpu"


#define MSG_LEN_MAX             (1024)
//--//
UINT32 SendCount = 0;

void *pclient;
iotx_conn_info_pt pconn_info;
iotx_mqtt_param_t mqtt_params;
iotx_mqtt_topic_info_t topic_msg;
char msg_pub[512];
char *msg_buf = NULL, *msg_readbuf = NULL;
char param[256];

uint8_t DeviceState = 1;
float g_X = 0;
float g_Y = 0;
float g_Z = 0;

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
void ReadMPUTask(void)
{
	  int ret = 0;
   	uint8_t Buffer[20];
		uint16_t value[10];
	
	  while(1)
		{
			   //读取设备数据                    

	
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
void MQTTBaiduTask(void)
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
		

    memset(&mqtt_params, 0x0, sizeof(mqtt_params));
		mqtt_params.port = 1883;	
		mqtt_params.host = "liteos.mqtt.iot.gz.baidubce.com";
		mqtt_params.client_id = "649cf54c800542"; 
    mqtt_params.username = "liteos/imu";
		mqtt_params.password = "WVpwvt1cnfSvJ2yZzvXls+lnnwZf37Vxv8esxU6kXlk=";
    mqtt_params.pub_key = NULL; //直连
    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;
    mqtt_params.handle_event.h_fp = mqtt_event_handle;
    mqtt_params.handle_event.pcontext = NULL;
		
	  printf("-----host=%s port=%d\r\n",mqtt_params.host,mqtt_params.port);
		printf("-----username=%s password=%s\r\n",mqtt_params.username,mqtt_params.password);
	
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
					   IOT_MQTT_Unsubscribe(pclient, IOT_TOPIC_MPU_POSTRSP);
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
					  DeviceState=2;					  
					  printf("MQTT construct succeed\r\n");
					  
				 }
			 }
			else if(DeviceState==2)
			{
					  //Initialize topic information
						memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
						topic_msg.qos = IOTX_MQTT_QOS1;
						topic_msg.retain = 0;
						topic_msg.dup = 0;
					
						//read sensor data
						memset(param, 0, sizeof(param));
						memset(msg_pub, 0, sizeof(msg_pub));	
						sprintf(param,  "{\"X\":%3.1f,\"Y\":%3.1f,\"Z\":%3.1f}",g_X,g_Y,g_Z);
										
						
						int msg_len = sprintf(msg_pub, param);
							 
						if (msg_len < 0) {
								printf("Error occur! Exit program\r\n");
								break;
						}
						printf("IOT_MQTT_Publish\r\n");			
						topic_msg.payload = (void *)msg_pub;
						topic_msg.payload_len = msg_len;        
						rc = IOT_MQTT_Publish(pclient, IOT_TOPIC_MPU_POST, &topic_msg);
						if (rc < 0) {
								printf("error occur when publish. %d\r\n", rc);
								IOT_MQTT_Unsubscribe(pclient, IOT_TOPIC_MPU_POSTRSP);
								LOS_TaskDelay(200);
							
								IOT_MQTT_Destroy(&pclient);
								pclient = NULL;					
								DeviceState = 1;   //重新连接
								//continue;
							
						}						
						printf("packet-id=%u, publish topic msg=%s\r\n", (uint32_t)rc, msg_pub);
						IOT_MQTT_Yield(pclient, 200);	

				
						LOS_TaskDelay(500);	///0.5秒上传一次数据		
									 				 
			}
			
      LOS_TaskDelay(20);			
    }   		
		
		//----------------------------------------//
    IOT_MQTT_Unsubscribe(pclient, IOT_TOPIC_MPU_POSTRSP);
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
UINT32 Create_ReadMPUTask()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "ReadMPUTask";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)ReadMPUTask;
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
UINT32 Create_MQTTBaiduTask()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "MQTTBaiduTask";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)MQTTBaiduTask;
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
	  
	  printf("-------------- LiteOS + Baidu IOT ---------------\r\n");  
		//Initilaize the LwIP stack
	  LwIP_Init();
	
		//设备状态+MQTT反馈信息任务
		uwRet = Create_ReadMPUTask();
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
		
		//MQTT
		uwRet = Create_MQTTBaiduTask();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
		
		//启动LiteOS
    LOS_Start();
}
