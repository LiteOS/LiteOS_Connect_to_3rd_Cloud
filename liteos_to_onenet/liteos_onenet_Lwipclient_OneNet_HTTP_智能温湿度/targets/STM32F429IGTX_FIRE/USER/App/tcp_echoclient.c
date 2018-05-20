/**
  ******************************************************************************
  * @file    tcp_echoclient.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013 
  * @brief   tcp echoclient application using LwIP RAW API
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "memp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netconf.h"
#include "LAN8742A.h"
#include "tcp_echoclient.h"
#include "cjson.h"
#include "bsp_led.h" 
#include  "utils.h"
#if LWIP_TCP
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

u8_t  recev_buf[50];
__IO uint32_t message_count=0;

u8_t   data[100];

extern struct tcp_pcb *echoclient_pcb;


/* ECHO protocol states */
enum echoclient_states
{
  ES_NOT_CONNECTED = 0,
  ES_CONNECTED,
  ES_RECEIVED,
  ES_CLOSING,
};


/* structure to be passed as argument to the tcp callbacks */
struct echoclient
{
  enum echoclient_states state; /* connection status */
  struct tcp_pcb *pcb;          /* pointer on the current tcp_pcb */
  struct pbuf *p_tx;            /* pointer on pbuf to be transmitted */
};

struct echoclient * echoclient_es;

/* Private function prototypes -----------------------------------------------*/
static err_t tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es);
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err);

/* Private functions ---------------------------------------------------------*/


/**
* @brief  Connects to the TCP echo server
* @param  None
* @retval None
*/
//tcp服务器连接状态
enum tcp_client_states
{
	ES_TCPCLIENT_NONE = 0,		//没有连接
	ES_TCPCLIENT_CONNECTED,		//连接到服务器了 
	ES_TCPCLIENT_CLOSING,		//关闭连接
}; 
typedef struct{
	char ucMsgBuf[1024];
	int dwLen;
}MSG_BUF_STRUCT;
//LWIP回调函数使用的结构体
struct tcp_client_struct
{
	u8 state;               //当前连接状
	struct tcp_pcb *pcb;    //指向当前的pcb
	struct pbuf *p;         //指向接收/或传输的pbuf
}; 
//TCP Client接收数据缓冲区
MSG_BUF_STRUCT tcp_client_recvbuf;
int tcp_client_flag;
//lwIP tcp_recv()函数的回调函数
//tcp_echoclient_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t tcp_echoclient_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{
    u32 data_len=0;
    struct pbuf *q;
    struct tcp_client_struct *es;
    err_t ret_err;
    LWIP_ASSERT("arg != NULL",arg != NULL);
    es=(struct tcp_client_struct *)arg;
    if(p==NULL)//如果从服务器接收到空的数据帧就关闭连接
    {
        es->state=ES_TCPCLIENT_CLOSING;//需要关闭TCP 连接了
        es->p=p;
        ret_err=ERR_OK;
    }
    else if(err!= ERR_OK) //当接收到一个非空的数据帧,但是err!=ERR_OK
    {
        if(p)pbuf_free(p);//释放接收pbuf
        ret_err=err;
    }
    else if(es->state==ES_TCPCLIENT_CONNECTED)	//当处于连接状态时
    {
        if(p!=NULL)//当处于连接状态并且接收到的数据不为空时
        {
            memset(tcp_client_recvbuf.ucMsgBuf,0,1024);  //数据接收缓冲区清零
            for(q=p; q!=NULL; q=q->next) //遍历完整个pbuf链表
            {
                //判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
                //的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
                if(q->len > (1024-data_len)) memcpy(tcp_client_recvbuf.ucMsgBuf+data_len,q->payload,(1024-data_len));//拷贝数据
                else memcpy(tcp_client_recvbuf.ucMsgBuf+data_len,q->payload,q->len);
                data_len += q->len;
                if(data_len > 1024) break; //超出TCP客户端接收数组,跳出
            }
						tcp_client_recvbuf.dwLen = data_len;
            tcp_client_flag=1;		//标记接收到数据了
            tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
						//printf("接收到%d字节数据:\n%s\n",tcp_client_recvbuf.dwLen,tcp_client_recvbuf.ucMsgBuf);
            pbuf_free(p);  	//释放内存
            ret_err=ERR_OK;
        }
    }
    else   //接收到数据但是连接已经关闭,
    {
        tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
        es->p=NULL;
        pbuf_free(p); //释放内存
        ret_err=ERR_OK;
    }
    return ret_err;
}
void tcp_echoclient_connect(void)
{
  struct ip_addr DestIPaddr;
		int ee;
  /* create new tcp pcb */
  echoclient_pcb = tcp_new();
  
  if (echoclient_pcb != NULL)
  {
    IP4_ADDR( &DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );
    
    /* connect to destination address/port */
    ee=tcp_connect(echoclient_pcb,&DestIPaddr,DEST_PORT,tcp_echoclient_connected);
     if(ERR_OK == ee)   // ?????
			 {
					 printf("\n\r Connect Server SUCCESS!");   
			 }
			 else {printf("\n\r Connect Server fail!"); }		
	
	}
  else
  {
    /* deallocate the pcb */
    memp_free(MEMP_TCP_PCB, echoclient_pcb);
#ifdef SERIAL_DEBUG
    printf("\n\r can not create tcp pcb");
#endif 
  }
}

/**
* @brief  Disconnects to the TCP echo server
* @param  None
* @retval None
*/
void tcp_echoclient_disconnect(void)
{
	/* close connection */
  tcp_echoclient_connection_close(echoclient_pcb,echoclient_es);
#ifdef SERIAL_DEBUG
    printf("\n\r close TCP connection");
#endif 
}

/**
  * @brief Function called when TCP connection established
  * @param tpcb: pointer on the connection contol block
  * @param err: when connection correctly established err should be ERR_OK 
  * @retval err_t: returned error 
  */
static err_t tcp_echoclient_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  struct echoclient *es = NULL;
  
  if (err == ERR_OK)   
  {
    /* allocate structure es to maintain tcp connection informations */
    es = (struct echoclient *)mem_malloc(sizeof(struct echoclient));
    echoclient_es=es;
    if (es != NULL)
    {
      es->state = ES_CONNECTED;
      es->pcb = tpcb;
      
      //sprintf((char*)data, "sending tcp client message %d", message_count);
        
      /* allocate pbuf */
     // es->p_tx = pbuf_alloc(PBUF_TRANSPORT, strlen((char*)data) , PBUF_POOL);
         
      if (es->p_tx)
      {       
        /* copy data to pbuf */
       // pbuf_take(es->p_tx, (char*)data, strlen((char*)data));
        
        /* pass newly allocated es structure as argument to tpcb */
        tcp_arg(tpcb, es);
  
        /* initialize LwIP tcp_recv callback function */ 
        tcp_recv(tpcb, tcp_echoclient_recv);
  
        /* initialize LwIP tcp_sent callback function */
        //tcp_sent(tpcb, tcp_echoclient_sent);
  
        /* initialize LwIP tcp_poll callback function */
        //tcp_poll(tpcb, tcp_echoclient_poll, 1);
    
        /* send data */
        //tcp_echoclient_send(tpcb,es);
        
        return ERR_OK;
      }
    }
    else
    {
      /* close connection */
      tcp_echoclient_connection_close(tpcb, es);
      
      /* return memory allocation error */
      return ERR_MEM;  
    }
  }
  else
  {
    /* close connection */
    tcp_echoclient_connection_close(tpcb, es);
  }
  return err;
}
    
/**
  * @brief tcp_receiv callback
  * @param arg: argument to be passed to receive callback 
  * @param tpcb: tcp connection control block 
  * @param err: receive error code 
  * @retval err_t: retuned error  
  */
char *recdata=0;
char temp[1556];
static err_t tcp_echoclient_recv11(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{ 
	
	
  struct echoclient *es;
  err_t ret_err;
  cJSON *json;

  LWIP_ASSERT("arg != NULL",arg != NULL);
  
  es = (struct echoclient *)arg;
	
  /* if we receive an empty tcp frame from server => close connection */
  if (p == NULL)
  {
    /* remote host closed connection */
    es->state = ES_CLOSING;
    if(es->p_tx == NULL)
    {
       /* we're done sending, close connection */
       tcp_echoclient_connection_close(tpcb, es);
    }
    else
    {    
      /* send remaining data*/
      tcp_echoclient_send(tpcb, es);
    }
    ret_err = ERR_OK;
  }   
  /* else : a non empty frame was received from echo server but for some reason err != ERR_OK */
  else if(err != ERR_OK)
  {
    /* free received pbuf*/
    pbuf_free(p);

    ret_err = err;
  }
	
  else if(es->state == ES_CONNECTED)
  {
    /* increment message count */
    message_count++;     
	
    /* Acknowledge data reception */
    tcp_recved(tpcb, p->tot_len);  
		printf("\r\nrec1:[%d]%d",sizeof(p->tot_len),p->tot_len);
#ifdef SERIAL_DEBUG
		recdata=malloc(p->tot_len*sizeof(unsigned char));
		printf("\r\nrec2:%s\r\n",recdata);
		if(recdata!=NULL)
		{
			memcpy(recdata,p->payload,p->tot_len);
		//	printf("\r\nrec:[%d]%s\r\n",p->tot_len,p->payload);
			printf("\r\nrec:[%d]%s",strlen(recdata),recdata);
			
		}
		
		json=cJSON_Parse(recdata);
		/*{"errno":0,"data":{"count":4,
		    "datastreams":[{
		                  "datapoints":[{
		                                "at":"2018-05-10 07:25:17.913","value":0
		                                }],
		                   "id":"LED1"},
		{"datapoints":[{"at":"2018-05-11 13:29:40.535","value":24}],"id":"temperature"}*/
			if (json != NULL)
			{
				cJSON *tmp;
				tmp = cJSON_GetObjectItem(json,"data");
				if(tmp != NULL)
				{
					tmp = cJSON_GetObjectItem(tmp,"datastreams");
					//printf("width=%d\n",tmp->valueint);
					if(tmp != NULL)
									{
										tmp = cJSON_GetObjectItem(tmp,"datapoints");
										//printf("width=%d\n",tmp->valueint);	
										if(tmp != NULL)
										{
											tmp = cJSON_GetObjectItem(tmp,"id");
											printf("id=%s\n",tmp->valuestring);	
											
										}	
									}					
					
				}
			}
		free(recdata);
#endif	
		
		/* free received pbuf*/
    pbuf_free(p);
   // tcp_echoclient_connection_close(tpcb, es);
    ret_err = ERR_OK;
  }

  /* data received when connection already closed */
  else
  {
    /* Acknowledge data reception */
    tcp_recved(tpcb, p->tot_len);
    free(tpcb);
    /* free pbuf and do nothing */
    pbuf_free(p);
    ret_err = ERR_OK;
  }
	
  return ret_err;
}

/**
  * @brief function used to send data
  * @param  tpcb: tcp control block
  * @param  es: pointer on structure of type echoclient containing info on data 
  *             to be sent
  * @retval None 
  */
static void tcp_echoclient_send(struct tcp_pcb *tpcb, struct echoclient * es)
{
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;
 
  while ((wr_err == ERR_OK) &&
         (es->p_tx != NULL) && 
         (es->p_tx->len <= tcp_sndbuf(tpcb)))
  {
    
    /* get pointer on pbuf from es structure */
    ptr = es->p_tx;

    /* enqueue data for transmission */
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
    
    if (wr_err == ERR_OK)
    { 
      /* continue with next pbuf in chain (if any) */
      es->p_tx = ptr->next;
      
      if(es->p_tx != NULL)
      {
        /* increment reference count for es->p */
        pbuf_ref(es->p_tx);
      }
      
      /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
      pbuf_free(ptr);
   }
   else if(wr_err == ERR_MEM)
   {
      /* we are low on memory, try later, defer to poll */
     es->p_tx = ptr;
   }
   else
   {		 
     /* other problem ?? */
   }
  }
}

/**
  * @brief  This function implements the tcp_poll callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: tcp connection control block
  * @retval err_t: error code
  */
static err_t tcp_echoclient_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct echoclient *es;

  es = (struct echoclient*)arg;
  if (es != NULL)
  {
    if (es->p_tx != NULL)
    {
      /* there is a remaining pbuf (chain) , try to send data */
      tcp_echoclient_send(tpcb, es);
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if(es->state == ES_CLOSING)
      {
        /* close tcp connection */
        tcp_echoclient_connection_close(tpcb, es);
      }
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * @param  arg: pointer on argument passed to callback
  * @param  tcp_pcb: tcp connection control block
  * @param  len: length of data sent 
  * @retval err_t: returned error code
  */
static err_t tcp_echoclient_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct echoclient *es;

  LWIP_UNUSED_ARG(len);

  es = (struct echoclient *)arg;
  
  if(es->p_tx != NULL)
  {
    /* still got pbufs to send */
    tcp_echoclient_send(tpcb, es);
  }

  return ERR_OK;
}

/**
  * @brief This function is used to close the tcp connection with server
  * @param tpcb: tcp connection control block
  * @param es: pointer on echoclient structure
  * @retval None
  */
static void tcp_echoclient_connection_close(struct tcp_pcb *tpcb, struct echoclient * es )
{
  /* remove callbacks */
  tcp_recv(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_poll(tpcb, NULL,0);

  if (es != NULL)
  {
    mem_free(es);
  }

  /* close tcp connection */
  tcp_close(tpcb);
  
}

int led_flag=0;
int len_post;
int temperature1=0;
int humidity1=0;
char post_data2[255]={0};//全局变量，在bsp_dht11.c的int mainDHT11(void)函数中
void MCU_to_TCP(void)
      {
				 struct tcp_pcb *cpcb;
					cpcb=echoclient_pcb;			
         //将数据发送出去
				if(echoclient_pcb!=NULL)
				{				
					tcp_write(echoclient_pcb,post_data2,strlen(post_data2),1);//接收到服务器数据后再反馈原有数据回服务器
					tcp_output(echoclient_pcb);                       //循环发送数据
				}	 			
		    else free(cpcb);
				free(post_data2);
      }
			
const unsigned char get_requst3[]="GET /devices/29702274/datapoints HTTP/1.1\r\napi-key:  TnWg0VdW6kIhU9=6I9AewR=TxKQ=\r\nHost: api.heclouds.com\r\n\r\n";
const char get_requst11[]={0x47,0x45,0x54,0x20,0x2F,0x64,0x65,0x76,0x69,0x63,0x65,0x73,0x2F,0x32,0x39,0x37,0x30,0x32,0x32,0x37,0x34,0x2F,0x64,0x61,0x74,0x61,0x70,0x6F,0x69,0x6E,0x74,0x73,0x20,0x48,0x54,
	0x54,0x50,0x2F,0x31,0x2E,0x31,0x0D,0x0A,0x61,0x70,0x69,0x2D,0x6B,0x65,0x79,0x3A,0x20,0x20,0x54,0x6E,0x57,0x67,0x30,0x56,0x64,0x57,0x36,0x6B,0x49,0x68,0x55,0x39,0x3D,0x36,0x49,
	0x39,0x41,0x65,0x77,0x52,0x3D,0x54,0x78,0x4B,0x51,0x3D,0x0D,0x0A,0x48,0x6F,0x73,0x74,0x3A,0x20,0x61,0x70,0x69,0x2E,0x68,0x65,0x63,0x6C,0x6F,0x75,0x64,0x73,0x2E,0x63,0x6F,0x6D,
	0x0D,0x0A,0x0D,0x0A,0x0D,0x0A};//该数据是get_regust3[]字符串的十六进制，这样发送数据效率高
void GETdata(void)
{
	struct tcp_pcb *cpcb;
			cpcb=echoclient_pcb;
		if(echoclient_pcb!=NULL)
		{		
			tcp_write(cpcb,get_requst11,sizeof(get_requst11),1);//接收到服务器数据
			tcp_output(cpcb);                                   //循环发送数据
		}	 
}
/*********************************************************
解析接收到的数据包，并添加应用控制
2018.5.12
*********************************************************/
int i=0;
char MsgBuf[1024];
int sum=0;
int flagsum=0;
	void recDataAnalyze(void)
	{
//		cJSON *json;
	  if(tcp_client_flag==1)
		{
			
		 	printf("\r\n接收到%d字节数据:\n%s\n",tcp_client_recvbuf.dwLen,tcp_client_recvbuf.ucMsgBuf);
			//处理数据
			strcpy(MsgBuf,&tcp_client_recvbuf.ucMsgBuf[177]);	//过滤HTTP头部信息，获取需要的数据
			printf("\r\n提取数据:\n%s\n",MsgBuf);
			/**以下解析数据有点简单粗暴，解析有点麻烦，但是确实最实用，效率比用cJSON高,占用内存较少,所以暂时不用cJSON**/
			for(i=0;i<strlen(MsgBuf);i++)
			{
				if(MsgBuf[i]=='v'&&(MsgBuf[i+1]=='a'&&MsgBuf[i+2]=='l'&&MsgBuf[i+3]=='u'&&MsgBuf[i+4]=='e'&&MsgBuf[i+5]=='"'))
					
				{
					 if(MsgBuf[i+6]==':'&&MsgBuf[i+7]=='1')
					 {
						if(MsgBuf[i+17]=='L'&&MsgBuf[i+18]=='E'&&MsgBuf[i+19]=='D'&&MsgBuf[i+20]=='1')
							{
								LED2_ON;i=0;
								memset(MsgBuf,0,1024);
							}
						}
					if(MsgBuf[i+6]==':'&&MsgBuf[i+7]=='0')
					{
						if(MsgBuf[i+17]=='L'&&MsgBuf[i+18]=='E'&&MsgBuf[i+19]=='D'&&MsgBuf[i+20]=='1')
							{
								LED2_OFF;i=0;
								memset(MsgBuf,0,1024);
							}
						}			
				}
	    }
		   tcp_client_flag=0;
		}
	
	}
#endif 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
