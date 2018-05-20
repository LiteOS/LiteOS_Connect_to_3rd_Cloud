#include "mqtt.h"
#include <string.h>
#include "utils.h"
#include "stm32f4xx.h"

#include "tcp.h"
#ifdef WIN32
#error Not support Windows now.
#endif // WIN32

#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <time.h>
#include "sys1.h"
#include "Bsp/systick/bsp_SysTick.h"
#include "Bsp/led/bsp_led.h" 
#include "Bsp/usart/bsp_debug_usart.h"
#include "sys1.h"
#include "bsp_key.h"
#include "tcp_echoclient.h"
#include "mqtt_loop.h"
#include "netconf.h"
#include "stm32f429_eth.h"
struct tcp_pcb *echoclient_pcb;
const char send_buf[MAX_MQTT_LEN];
struct MqttSampleContext smpctx[1];
u8 ir_state = 0xff;
int key_event = 0;

struct Command
{
    const char *cmd;
    int (*func)(struct MqttSampleContext *ctx);
    const char *desc;
};
void set_key_event(int event);
void process_key_event(void);
static int MqttSample_CmdConnect(struct MqttSampleContext *ctx);
static int MqttSample_CmdSubscribe(struct MqttSampleContext *ctx);
#if 0
static int MqttSample_CmdPing(struct MqttSampleContext *ctx);
static int MqttSample_CmdPublish(struct MqttSampleContext *ctx);

static int MqttSample_CmdUnsubscribe(struct MqttSampleContext *ctx);
static int MqttSample_CmdDisconnect(struct MqttSampleContext *ctx);
static int MqttSample_CmdCmdRet(struct MqttSampleContext *ctx);
static int MqttSample_CmdExit(struct MqttSampleContext *ctx);


static const struct Command commands[] =
{
    {"connect", MqttSample_CmdConnect, "Establish the connection."},
    {"ping", MqttSample_CmdPing, "Send ping packet."},
    {"publish", MqttSample_CmdPublish, "Send data points."},
    {"subscribe", MqttSample_CmdSubscribe, "Subscribe the data streams."},
    {"unsubscribe", MqttSample_CmdUnsubscribe, "Unsubscribe the data streams."},
    {"disconnect", MqttSample_CmdDisconnect, "Close the connection."},
    {"cmdret", MqttSample_CmdCmdRet, "Sed the command returen information."},
    {"exit", MqttSample_CmdExit, "Exit the sample."}
};
#endif
static int MqttSample_RecvPkt(unsigned char *buf)
{
    int bytes;
    int timeout = 200;
   struct pbuf *p;
    int i = 0;
    char *recdata;
    while(1)
    {
        Delay_10ms(100);
        bytes = 1024;//USART2_GetRcvNum();
        //USART2_GetRcvData(buf, bytes);
        if(bytes > 0)
        {
					  Delay_10ms(50);
						bytes = 1024;//USART2_Get
					/* Acknowledge data reception */
        tcp_recved(echoclient_pcb, p->tot_len);  
		    //tcp_write(tpcb,p->payload,p->tot_len,1);//接收到服务器数据后再反馈原有数据回服务器
		    //tcp_output(tpcb);                       //循环发送数据RcvNum();
					
						recdata=(char *)malloc(p->len*sizeof(char));
					if(recdata!=NULL)
					{
						memcpy(recdata,p->payload,p->len);
						printf("\r\nrec<<<<<\r\n%s",recdata);
					}
            break;
        }
        if(timeout == 0)
        {

            break;
        }
        timeout--;
    }
    printf("bytes=%d\n", bytes);
    if(bytes > 0)
    {
        printf("\r\n----------\r\n");
        for(i = 0; i < bytes; i++)
        {
            printf("0x%x ", ((unsigned char *)buf)[i]);
        }
        printf("\r\n----------\r\n");
    }
    return bytes;
}

static int MqttSample_SendPkt(void *arg, const struct iovec *iov, int iovcnt)
{
//   struct pbuf *p;
    int bytes = 0;
    int i = 0, j = 0;
    struct iovec *tmp;
    memset((void*)send_buf, 0, MAX_MQTT_LEN);
    for(i = 0; i < iovcnt; i++)
    {
        tmp = (struct iovec *)(&iov[i]);
        bytes += tmp->iov_len;
    }

    if(bytes > MAX_MQTT_LEN)
    {
        printf("%s too much data to send\n", __func__);
        return -1;
    }

    for(i = 0; i < iovcnt; i++)
    {
        tmp = (struct iovec *)(&iov[i]);
        memcpy((void *)(send_buf + j), tmp->iov_base,  tmp->iov_len);
        j += tmp->iov_len;
    }

    printf("\r\n*****bytes=%d******\r\n",bytes);
    for(i=0;i<bytes;i++)
    {
      printf("0x%x ",send_buf[i]);
    }
    printf("\r\n*********\r\n");
    //bytes = USART2_SendData((int8_t *)send_buf, bytes);
		 //tcp_write((int8_t *)send_buf, arg, bytes, 1);
		  tcp_write(echoclient_pcb,send_buf,strlen(send_buf),1);//接收到服务器数据后再反馈原有数据回服务器
		  tcp_output(echoclient_pcb);                       //循环发送数据RcvNum();
		if(echoclient_pcb!=NULL)
				{
	
					printf("\r\nUSART:%s",send_buf);				
					tcp_write(echoclient_pcb,send_buf,strlen(send_buf),1);//接收到服务器数据后再反馈原有数据回服务器
					tcp_output(echoclient_pcb);                       //循环发送数据					
					//tcp_write(echoclient_pcb,get_requst3,sizeof(get_requst3),1);//接收到服务器数据后再反馈原有数据回服务器
					//tcp_output(echoclient_pcb);                       //循环发送数据
					
					//	flagsend=0;
				}	 
		 //tcp_echoclient_send(((int8_t *)send_buf, bytes);
    return bytes;
}

//------------------------------- packet handlers -------------------------------------------
static int MqttSample_HandleConnAck(void *arg, char flags, char ret_code)
{
    printf("Success to connect to the server, flags(%d), code(%d).\n",
           flags, ret_code);
    return 0;
}

static int MqttSample_HandlePingResp(void *arg)
{
    printf("Recv the ping response.\n");
    return 0;
}

static int MqttSample_HandlePublish(void *arg, uint16_t pkt_id, const char *topic,
                                    const char *payload, uint32_t payloadsize,
                                    int dup, enum MqttQosLevel qos)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    ctx->dup = dup;
    ctx->qos = qos;
    printf("dup=%d, qos=%d, id=%d\ntopic: %s\npayloadsize=%d\n",
           dup, qos, pkt_id, topic, payloadsize);
    return 0;
}

static int MqttSample_HandlePubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the publish ack, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubRec(void *arg, uint16_t pkt_id)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    printf("Recv the publish rec, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubRel(void *arg, uint16_t pkt_id)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    printf("Recv the publish rel, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubComp(void *arg, uint16_t pkt_id)
{

    printf("Recv the publish comp, packet id is %d.\n", pkt_id);
    printf("%s %d\n", __func__, __LINE__);
    printf("!!!!!!!!!!!!!!!!!!\n");
    smpctx->publish_state = 0;
    return 0;
}

static int MqttSample_HandleSubAck(void *arg, uint16_t pkt_id, const char *codes, uint32_t count)
{
    uint32_t i;
    printf("%s %d\n", __func__, __LINE__);
    printf("Recv the subscribe ack, packet id is %d, return code count is %d:.\n", pkt_id, count);
    for(i = 0; i < count; ++i)
    {
        printf("   code%d=%02x\n", i, codes[i]);
    }

    return 0;
}

static int MqttSample_HandleUnsubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the unsubscribe ack, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandleCmd(void *arg, uint16_t pkt_id, const char *cmdid,
                                int64_t timestamp, const char *desc, const char *cmdarg,
                                uint32_t cmdarg_len, int dup, enum MqttQosLevel qos)
{
    uint32_t i;
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    strcpy(ctx->cmdid, cmdid);
    printf("Recv the command, packet id is %d, cmdid is %s, qos=%d, dup=%d.\n",
           pkt_id, cmdid, qos, dup);

    if(0 != timestamp)
    {
        time_t seconds = timestamp / 1000;
        struct tm *st = localtime(&seconds);

        printf("    The timestampe is %04d-%02d-%02dT%02d:%02d:%02d.%03d.\n",
               st->tm_year + 1900, st->tm_mon + 1, st->tm_mday,
               st->tm_hour, st->tm_min, st->tm_sec, (int)(timestamp % 1000));
    }
    else
    {
        printf("    There is no timestamp.\n");
    }

    if(NULL != desc)
    {
        printf("    The description is: %s.\n", desc);
    }
    else
    {
        printf("    There is no description.\n");
    }

    printf("    The length of the command argument is %d, the argument is:", cmdarg_len);
    for(i = 0; i < cmdarg_len; ++i)
    {
        const char c = cmdarg[i];
        if(0 == i % 16)
        {
            printf("\n        ");
        }

        printf("%02X'%c' ", c, c);
    }
    printf("\n");

    return 0;
}

/**
  * @brief ??MQTT Connet??,????
    * @param  ctx:?????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdConnect(struct MqttSampleContext *ctx)
{
    int err;

    printf("%s %d\n", __func__, __LINE__);
    err = Mqtt_PackConnectPkt(ctx->mqttbuf, 0, ctx->devid, 1, "WillTopic",
                              "will message-test", 17, MQTT_QOS_LEVEL0, 0, ctx->proid,
                              ctx->apikey, strlen(ctx->apikey));
   if(MQTTERR_NOERROR != err)
    {
        printf("Failed to pack the MQTT CONNECT PACKET, errcode is %d.\n", err);
        return -1;
    } /**/
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}
/*?????????,????*/
#if 0
/**
  * @brief ??MQTT Ping??
    * @param  ctx:?????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdPing(struct MqttSampleContext *ctx)
{
    int err;

    printf("%s %d\n", __func__, __LINE__);
    err = Mqtt_PackPingReqPkt(ctx->mqttbuf);
    if(MQTTERR_NOERROR != err)
    {
        printf("Critical bug: failed to pack the ping request packet.err=%d\n", err);
        return -1;
    }
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}
/**
  * @brief ???????,??????,????JSON??,???????Json??,???????
  * @param  ctx:?????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdPublish(struct MqttSampleContext *ctx)
{
    uint8_t cause = 0; //only for test
    int err = 0;

    static unsigned char count = 0;
    int64_t ts = 0; //no time
    uint16_t temprature[1], rh[1];
    printf("%s %d,count=%d,cause=%d\n", __func__, __LINE__, count, cause);
    // ts = (int64_t)time(NULL) * 1000;

    SHT2x_MeasureHM(SHT20_Measurement_RH_HM, temprature);
    Delay_10ms(1500);
    SHT2x_MeasureHM(SHT20_Measurement_T_HM, rh);

    err |= Mqtt_PackDataPointStart(ctx->mqttbuf, 1, MQTT_QOS_LEVEL2, 0, 1);
    if((count == 0) || (cause == EVENT))
    {
        err |= Mqtt_AppendDPStartObject(ctx->mqttbuf, DS_TO_PUBLISH, ts);
        if(cause == TIME_OUT)
        {
            ir_state = !ir_state;
        }
        if(ir_state == 0x1)
        {
            err |= Mqtt_AppendDPSubvalueString(ctx->mqttbuf, DS_TO_PUBLISH, "????");
        }
        else if(ir_state == 0)
        {
            err |= Mqtt_AppendDPSubvalueString(ctx->mqttbuf, DS_TO_PUBLISH, "????");
        }
        else
        {
            err |= Mqtt_AppendDPSubvalueString(ctx->mqttbuf, DS_TO_PUBLISH, "??");
        }
        if(count != 2)
        {
            count++;
        }
    }
    else if(count == 1)
    {
        err |= Mqtt_AppendDPStartObject(ctx->mqttbuf, DS_TO_PUBLISH_T, ts);
        err |= Mqtt_AppendDPSubvalueInt(ctx->mqttbuf, DS_TO_PUBLISH_T, temprature[0]);
        count++;
    }
    else if(count == 2)
    {
        err |= Mqtt_AppendDPStartObject(ctx->mqttbuf, DS_TO_PUBLISH_RH, ts);
        err |= Mqtt_AppendDPSubvalueInt(ctx->mqttbuf, DS_TO_PUBLISH_RH, rh[0]);
        count = 0;

    }
    err |= Mqtt_AppendDPFinishObject(ctx->mqttbuf);
    err |= Mqtt_PackDataPointFinish(ctx->mqttbuf);
    if(err)
    {
        printf("Failed to pack data point package.err=%d\n", err);
        return -1;
    }
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}
#endif
int Mqtt_AppendLength(struct MqttBuffer *buf, uint32_t len);

/**
  * @brief ???????,???????,????????????,?????????
  * @param  ctx:?????
    * @param  cause:????,TIMEOUT,????,EVENT?????????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdPublishNormal(struct MqttSampleContext *ctx, uint8_t cause)
{
    int err = 0, ir_index = 0;

    struct MqttExtent *ext;
    unsigned char dis[3][64] = {"未知","设备在位","设备离位"};
    static unsigned char count = 0;

    uint16_t temprature[1], rh[1];
    if(ctx->publish_state != 0)
    {
        printf("publsh busy\n");
        return -1;
    }
    ctx->publish_state = 1;
    printf("%s %d,count=%d,cause=%d\n", __func__, __LINE__, count, cause);
    // ts = (int64_t)time(NULL) * 1000;
   // SHT2x_MeasureHM(SHT20_Measurement_RH_HM, temprature);
    Delay_10ms(1500);
   // SHT2x_MeasureHM(SHT20_Measurement_T_HM, rh);

    if((count == 0) || (cause == EVENT))
    {
        //no time
        if(cause == TIME_OUT)
        {
            ir_state = !ir_state;
        }
        if(ir_state == 0xff)
        {
            ir_index = 0;
        }
        if(ir_state == 0x1)
        {
            ir_index = 1;
        }
        if(ir_state == 0)
        {
            ir_index = 2;

        }
        if(count != 2)
        {
            count++;
        }
        //append data; String
        printf("pub state\n");
        err |=  Mqtt_PackDataPointStartNormal(ctx->mqttbuf, (unsigned char *)(DS_TO_PUBLISH), 1, MQTT_QOS_LEVEL2, 0, 1);
        ext = MqttBuffer_AllocExtent(ctx->mqttbuf, 1 + strlen((const char *)dis[ir_index]));
        if(!ext)
        {
            return MQTTERR_OUTOFMEMORY;
        }
        ext->payload[0] = 0x84;;
        memcpy(&ext->payload[1], dis[ir_index], strlen((const char *)dis[ir_index]));
        MqttBuffer_AppendExtent(ctx->mqttbuf, ext);
    }
    else if(count == 1)
    {
        printf("pub temprature\n");
        err |= Mqtt_PackDataPointStartNormal(ctx->mqttbuf, (unsigned char *)(DS_TO_PUBLISH_T), 1, MQTT_QOS_LEVEL2, 0, 1);
        //append data; int32
        ext = MqttBuffer_AllocExtent(ctx->mqttbuf, 1 + sizeof(int));
        if(!ext)
        {
            return MQTTERR_OUTOFMEMORY;
        }
        ext->payload[0] = 0x81;
        ext->payload[1] = 0;
        ext->payload[2] = 0;
        ext->payload[3] = 0;
        ext->payload[4] = temprature[0];
        MqttBuffer_AppendExtent(ctx->mqttbuf, ext);
        count++;
    }
    else if(count == 2)
    {
        printf("rh\n");
        err |= Mqtt_PackDataPointStartNormal(ctx->mqttbuf, (unsigned char *)(DS_TO_PUBLISH_RH), 1, MQTT_QOS_LEVEL2, 0, 1);
        //append data; int32
        ext = MqttBuffer_AllocExtent(ctx->mqttbuf, 1 + sizeof(int));
        if(!ext)
        {
            return MQTTERR_OUTOFMEMORY;
        }
        ext->payload[0] = 0x81;;
        ext->payload[1] = 0;
        ext->payload[2] = 0;
        ext->payload[3] = 0;
        ext->payload[4] = rh[0];
        MqttBuffer_AppendExtent(ctx->mqttbuf, ext);
        count = 0;
    }

    if(err)
    {
        printf("Failed to pack data point package.err=%d\n", err);
        return -1;
    }
    err = Mqtt_AppendLength(ctx->mqttbuf, ext->len);
    if(MQTTERR_NOERROR != err)
    {
        return err;
    }

    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}

/**
  * @brief ??????,TOPIC?TOPIC_TO_SUB
  * @param  ctx:?????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdSubscribe(struct MqttSampleContext *ctx)
{
    int err;

    printf("%s %d\n", __func__, __LINE__);
    err = Mqtt_PackSubscribePkt(ctx->mqttbuf, 1, TOPIC_TO_SUB, MQTT_QOS_LEVEL1);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the subscribe packet.\n");
        return -1;
    }

    printf("%s %d\n", __func__, __LINE__);
    /* err = Mqtt_AppendSubscribeTopic(ctx->mqttbuf, "433223/Bs04OCJioNgpmvjRphRak15j7Z8=/25267/test-2", MQTT_QOS_LEVEL2); */
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to append the topic to the "
               "subscribe packet.\n");
        return -1;
    }
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}
/*??????????api,????*/
#if 0
/**
  * @brief ??????,TOPIC?TOPIC_TO_UNSUB
  * @param  ctx:?????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdUnsubscribe(struct MqttSampleContext *ctx)
{
    int err;

    printf("%s %d\n", __func__, __LINE__);
    err = Mqtt_PackUnsubscribePkt(ctx->mqttbuf, PACK_FALG_UNSUB, TOPIC_TO_UNSUB);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the unsubscribe packet.\n");
        return -1;
    }

    err = Mqtt_AppendUnsubscribeTopic(ctx->mqttbuf, TOPIC_TO_UNSUB);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to append the topic to the "
               "unsubscribe packet.\n");
        return -1;
    }
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}


static int MqttSample_CmdDisconnect(struct MqttSampleContext *ctx)
{
    int err;
    printf("%s %d\n", __func__, __LINE__);
    err = Mqtt_PackDisconnectPkt(ctx->mqttbuf);
    if(MQTTERR_NOERROR != err)
    {
        printf("Critical bug: failed to pack the disconnect packet.\n");
        return -1;
    }
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 1;
}

static int MqttSample_CmdCmdRet(struct MqttSampleContext *ctx)
{
    int err;

    printf("%s %d\n", __func__, __LINE__);
    err = Mqtt_PackCmdRetPkt(ctx->mqttbuf, 1, ctx->cmdid,
                             "dkdkkxiiii", 11, 0);
    if(MQTTERR_NOERROR != err)
    {
        printf("Critical bug: failed to pack the cmd ret packet.\n");
        return -1;
    }
    Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    return 0;
}
/**
  * @brief ??MQTT???,????,???????,?????
  * @param  ctx:?????
  * @retval ret:0,??,<0 ??
  **/
static int MqttSample_CmdExit(struct MqttSampleContext *ctx)
{
    (void)ctx;
    return -1;
}
#endif
/**
  * @brief  初始化MQTT上下文，开发者主要替换设备ID,项目ID和API_KEY
  * @param  ctx:上下文变量
  * @retval ret:0,成功，<0失败
  **/
static int MqttSample_Init(struct MqttSampleContext *ctx)
{
    int err;

    ctx->sendedbytes = 0;
    ctx->proid = MQTT_DEVICE_PROJ_ID;
    ctx->devid = MQTT_DEVICE_ID;
    ctx->apikey = MQTT_DEVICE_API_KEY;
    ctx->publish_state = 0;
    err = Mqtt_InitContext(ctx->mqttctx, 1 << 8);
    if(MQTTERR_NOERROR != err)
    {
        printf("Failed to init the MQTT context errcode is %d", err);
        return -1;
    }

   // ctx->mqttctx->read_func = MqttSample_RecvPkt;
    ctx->mqttctx->writev_func = MqttSample_SendPkt;

    ctx->mqttctx->handle_conn_ack = MqttSample_HandleConnAck;
    ctx->mqttctx->handle_conn_ack_arg = ctx;
    ctx->mqttctx->handle_ping_resp = MqttSample_HandlePingResp;
    ctx->mqttctx->handle_ping_resp_arg = ctx;
    ctx->mqttctx->handle_publish = MqttSample_HandlePublish;
    ctx->mqttctx->handle_publish_arg = ctx;
    ctx->mqttctx->handle_pub_ack = MqttSample_HandlePubAck;
    ctx->mqttctx->handle_pub_ack_arg = ctx;
    ctx->mqttctx->handle_pub_rec = MqttSample_HandlePubRec;
    ctx->mqttctx->handle_pub_rec_arg = ctx;
    ctx->mqttctx->handle_pub_rel = MqttSample_HandlePubRel;
    ctx->mqttctx->handle_pub_rel_arg = ctx;
    ctx->mqttctx->handle_pub_comp = MqttSample_HandlePubComp;
    ctx->mqttctx->handle_pub_comp_arg = ctx;
    ctx->mqttctx->handle_sub_ack = MqttSample_HandleSubAck;
    ctx->mqttctx->handle_sub_ack_arg = ctx;
    ctx->mqttctx->handle_unsub_ack = MqttSample_HandleUnsubAck;
    ctx->mqttctx->handle_unsub_ack_arg = ctx;
    ctx->mqttctx->handle_cmd = MqttSample_HandleCmd;
    ctx->mqttctx->handle_cmd_arg = ctx;

    ctx->cmdid[0] = '\0';
    MqttBuffer_Init(ctx->mqttbuf);
    return 0;
}
/**
  * @brief  MQTT???????
  * @retval None
  **/	
uint8_t flag=0;
__IO uint32_t LocalTime=0; 
void MQTT_Loop(void)
{
    static int timeout_count = 0;
    int publish_state = 0;
    int exit = 0;
    int j = 0;
  
	
    /*初始化MQTT上下文,在该api内部配置了设备ID等信息*/
    if(MqttSample_Init(smpctx) < 0)
    {
        return;
    }
    printf("\r\ndevice id:%s\r\n", MQTT_DEVICE_ID);
    printf("we publish topic:%s %s %s\r\n",DS_TO_PUBLISH,DS_TO_PUBLISH_T,DS_TO_PUBLISH_RH);
    printf("connect to mqtt server\r\n");
    /*发送MQTT连接请求报文*/
    MqttSample_CmdConnect(smpctx);
		printf("\r\n");
    Delay_10ms(2000);
    Mqtt_RecvPkt(smpctx->mqttctx);
    Delay_10ms(2000);
     /*发送MQTT订阅topic报文*/
    MqttSample_CmdSubscribe(smpctx);//开机订阅
		printf("\r\n");
    Delay_10ms(2000);
    Mqtt_RecvPkt(smpctx->mqttctx);
    printf("begin loop\n");
    /*进入主循环*/
    while(!exit)
    {
			if((Key_Scan(KEY2_GPIO_PORT,KEY2_PIN)==KEY_ON) && flag)
				{
					//LED2_OFF;
					//LED33=~LED33;
					tcp_echoclient_disconnect();
					flag=0;
					Delay_10ms(30);
					break;
				}
        j++;
        if(j == 3)
        {
            //MqttSample_CmdPublish(smpctx,TIME_OUT);
             /*周期性发布温湿度和一个状态数据，状态每次取反*/
            publish_state = MqttSample_CmdPublishNormal(smpctx, TIME_OUT);
            if(publish_state < 0)
            {
                timeout_count++;
                if(timeout_count == 5)
                {
                    timeout_count = 0;
                    /*you can do some thing*/
                }
            }
            j = 0;
        }
        if(Mqtt_RecvPkt(smpctx->mqttctx)<0)
				{
					 smpctx->publish_state=0;
				}
				printf("\r\n");
        //Delay_10ms(100);
				/* check if any packet received */
       if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);
    }
    /*reclaim the resource 防止内存泄漏*/
    MqttBuffer_Destroy(smpctx->mqttbuf);
    Mqtt_DestroyContext(smpctx->mqttctx);
    printf("bye...\n");
    return;
}
