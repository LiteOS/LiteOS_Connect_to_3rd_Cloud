/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"
#include "iot_export.h"
#include "bsp_led.h" 
/** modbus */
#include "../../modbus.h"

// Aliyun IOT Advance
//#define PRODUCT_KEY             "a1FmhrCH03E"
//#define DEVICE_NAME             "Motor"
//#define DEVICE_SECRET           "Ax5igSM4D8Thnd6Lo9pfroG4aoCmIuMw"

// Aliyun IOT Base
#define PRODUCT_KEY             "a1TkfITEEOu"
#define DEVICE_NAME             "Tian_Motor"
#define DEVICE_SECRET           "LrrqLQboBEOwzVC52BB0MzEd3tuIxu9R"

/* These are pre-defined topics */
//#define TOPIC_UPDATE            "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/model/up_raw"
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update/state"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA              "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define MSG_LEN_MAX             (1024)

#define MQTT_EXAMPLE_TRACE
#ifdef MQTT_EXAMPLE_TRACE
#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        printf("%s|%03d :: ", __func__, __LINE__); \
        printf(fmt, ##args); \
        printf("%s", "\r\n"); \
    } while(0)
#else
#define EXAMPLE_TRACE(fmt, args...)
#endif // MQTT_EXAMPLE_TRACE

/** MQTT Global Variable */
void *pclient;
iotx_conn_info_pt pconn_info;
iotx_mqtt_param_t mqtt_params;
iotx_mqtt_topic_info_t topic_msg;
char msg_pub[128];    
char *msg_buf = NULL, *msg_readbuf = NULL;

/***/
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

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
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
}

extern volatile u8_t uDeviceStateDHCP;  /** 为1时表示通过 DHCP get到 ip */
extern volatile u8_t uDeviceStateDNS;   /** 为1时表示已经得到 Aliyun MQTT server 的 IP */
int mqtt_client(void)
{
    int rc = 0, msg_len;

    memset(msg_pub, 0x00, sizeof(msg_pub));

    if (NULL == (msg_buf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
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
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /** @2018-05-16 MQTT 连接之前先要检查 DHCP 和 DNS ok */
    do{        
        if(uDeviceStateDHCP)// && uDeviceStateDNS)
            break;
        else
            LOS_TaskDelay(1000);                   
    }while(1);
    printf("DHCP ok, then begin to Construct a MQTT client... \r\n");

do_ReMqttConstruct:
    /* Construct a MQTT client with specify parameter */    
    LOS_TaskDelay(1000);        
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed, and do_ReMqttConstruct...\r\n");
        rc = -1;
        //goto do_exit;
        goto do_ReMqttConstruct; /** 如果 Mqtt Construct 失败, 则再次 do_ReMqttConstruct */
    }

#if 0
    /** 目前无需订阅消息 */
    /** Subscribe the specific topic -- 当程序run到此处时, mqtt construct 必须已经是成功的 */
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_GET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }
#endif //#if

    LOS_TaskDelay(1000);

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t)); /** 之后就无需再 清空 topic_msg */
    memset(msg_pub, 0, sizeof(msg_pub)); /** 清空 msg_pub 消息内容 buffer */

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);
    
    do {
        LED1_OFF; LED2_OFF; LED3_ON;
        if( GetMotorStatusNow() != GetMotorStatusLast()) /** 电动机状态发生变化就需 update */
        {                        
            /** 先更新一下电动机的上一次的状态 */
            SetMotorStatusLast(GetMotorStatusNow());
            
            /* Generate topic message */
            memset(msg_pub, 0, sizeof(msg_pub)); // 清空
            msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"motor_state\", \"attr_value\":\"%d\"}", GetMotorStatusNow());
            if (msg_len < 0) {
                EXAMPLE_TRACE("Error occur! Exit program");
                rc = -1;
                break;
            }

            topic_msg.payload = (void *)msg_pub;
            topic_msg.payload_len = msg_len;

            rc = IOT_MQTT_Publish(pclient, TOPIC_UPDATE, &topic_msg);
            if (rc < 0) {
                EXAMPLE_TRACE("error occur when publish, then Release and ReMqttConstruct");
                IOT_MQTT_Destroy(&pclient);
                goto do_ReMqttConstruct; /** publish 失败则重新连接 */
            }
            EXAMPLE_TRACE("IOT_MQTT_Publish() : packet-id=%u, publish topic msg=%s", (uint32_t)rc, msg_pub);
        }
        
        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient,1000); /** @2018-05-16 Change 200 to 1000 */
        LED1_OFF; LED2_OFF; LED3_OFF;
        LOS_TaskDelay(2000);    // delay 5000 tick measn 5000ms
    } while(1);

    //IOT_MQTT_Unsubscribe(pclient, TOPIC_UPDATE);

    LOS_TaskDelay(200);

    IOT_MQTT_Destroy(&pclient);

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;
}

