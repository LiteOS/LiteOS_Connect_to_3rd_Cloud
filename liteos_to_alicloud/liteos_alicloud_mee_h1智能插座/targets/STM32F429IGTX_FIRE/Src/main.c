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
#include "cmsis_os.h"
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "bsp_led.h" 
#include "bsp_debug_usart.h"
#include "dwt.h"
#include "bsp_key.h"

#include "include.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        printf("%s|%03d :: ", __func__, __LINE__); \
        printf(fmt, ##args); \
        printf("%s", "\r\n"); \
    } while(0)




/* These are pre-defined topics */
#define PRODUCT_KEY             "a1xqhyNLtcU"
#define DEVICE_NAME             "MEE-H1"
#define DEVICE_SECRET           "uhCxhmb1oh1qO4ocqxMsB4kgBEeFUMBd"

//{"method":"thing.event.property.post","id":"2008001","params":{"PowerSwitch":1},"version":"1.0.0"}
#define ALINK_BODY_FORMAT         "{\"method\":\"%s\",\"id\":\"%d\",\"params\":{\"PowerSwitch\":%d},\"version\":\"1.0\"}"
#define ALINK_TOPIC_PROP_POST     "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"
#define ALINK_TOPIC_PROP_POSTRSP  "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post_reply"
#define ALINK_TOPIC_PROP_SET      "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#define ALINK_METHOD_PROP_POST    "thing.event.property.post"

#define MQTT_MSGLEN             (1024)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle;
UINT32 g_TskStart;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
char UserSwitch;
    
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void hardware_init(void)
{
	LED_GPIO_Config();
	Key1_GPIO_Config();
	Key2_GPIO_Config();
	KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
	Debug_USART_Config();
	DelayInit(SystemCoreClock);
	printf("Sysclock is %d\r\n",SystemCoreClock);
	ETH_BSP_Config();//以太网配置
}


void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}


static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    /* print topic name and topic message */
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    EXAMPLE_TRACE("----");

    if(ptopic_info->topic_len==50)
    {
        const char *inf = "\"PowerSwitch\":";
        char *dat = strstr(ptopic_info->payload, inf);
        
        dat += strlen(inf);

        UserSwitch = dat[0] - '0';

        if(UserSwitch==0)
        {
            LED_RGBOFF;
        }
        else
        {
            LED_BLUE;
        }
    }
}


int mqtt_client(void)
{
    extern __IO uint8_t DHCP_state;
    int rc = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;
    char out;

    while(DHCP_state != DHCP_ADDRESS_ASSIGNED)
    {
        HAL_SleepMs(1000);
    }

    if (NULL == (msg_buf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));
    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = NULL;
    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;
    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Construct a MQTT client with specify parameter */
    out = 0;
    do{
        pclient = IOT_MQTT_Construct(&mqtt_params);
        HAL_SleepMs(2000);
    }while((NULL==pclient)&&(++out<3));
    if (out >= 3) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Subscribe the specific topic */
    out = 0;
    do{
        rc = IOT_MQTT_Subscribe(pclient, ALINK_TOPIC_PROP_POSTRSP, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
        if(rc < 0) IOT_MQTT_Destroy(&pclient);
    }while((rc < 0)&&(++out<3));
    if (rc < 0) {
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() ACK failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }
    HAL_SleepMs(1000);

    out = 0;
    do{
        rc = IOT_MQTT_Subscribe(pclient, ALINK_TOPIC_PROP_SET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
        if(rc < 0) IOT_MQTT_Destroy(&pclient);
    }while((rc < 0)&&(++out<3));
    if (rc < 0) {
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() UP failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }
    HAL_SleepMs(1000);   

    while(1)
    {
        /* Initialize topic information */
        memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
        topic_msg.qos = IOTX_MQTT_QOS1;
        topic_msg.retain = 0;
        topic_msg.dup = 0;

        {
            //{"method":"thing.event.property.post","id":"2008001","params":{"PowerSwitch":1},"version":"1.0.0"}
            static int serial;
            sprintf(msg_pub, ALINK_BODY_FORMAT,ALINK_METHOD_PROP_POST, ++serial,UserSwitch);
        }
        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = strlen(msg_pub);

        rc = IOT_MQTT_Publish(pclient, ALINK_TOPIC_PROP_POST, &topic_msg);
        
        EXAMPLE_TRACE("rc = IOT_MQTT_Publish() = %d", rc);

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        IOT_MQTT_Unsubscribe(pclient, ALINK_TOPIC_PROP_POSTRSP);

        HAL_SleepMs(2000);
    }

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;
}

VOID task_start()
{
    TSK_INIT_PARAM_S task_init_param;
    
    //创建DHCP任务
    printf("Creat Task DHCP...\r\n");
    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "DHCP";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)LwIP_DHCP_task;
    task_init_param.uwStackSize = 0x400;
    LOS_TaskCreate(&g_TskHandle, &task_init_param);

    //创建mqtt任务
    printf("Creat Task mqtt_client...\r\n");
    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "mqtt_client";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)mqtt_client;
    task_init_param.uwStackSize = 0x4000;
    LOS_TaskCreate(&g_TskHandle, &task_init_param);    

    //删除任务
    LOS_TaskDelete(g_TskStart);
}

UINT32 creat_start()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 30;
    task_init_param.pcName = "start";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task_start;
    task_init_param.uwStackSize = 0x400;
    
    uwRet = LOS_TaskCreate(&g_TskStart, &task_init_param);
    
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}

int main(void)
{
    LOS_KernelInit();//内核初始化	
    
    hardware_init();//硬件初始化

    printf("LwIP_Init...\r\n");
    LwIP_Init();   //初始化协议栈

    creat_start();//创建开始任务

    LOS_Start();//启动LiteOS,Run!Run!Run!    
}
