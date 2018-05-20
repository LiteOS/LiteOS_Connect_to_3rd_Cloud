#define _TCP_CLIENT_DEMO_C_

#include "tcp_client_demo.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "./Bsp/usart/bsp_debug_usart.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "lb_timer.h"
#include "./Bsp/led/bsp_led.h"   
#include "Wdt.h"



/*
*功能：STM32F407VET6以太网核心板测试代码，自动获取IP，并建立服务器，可通过ITM的串口打印当前数据信息
*			 1.关于keil5如何安装，视频教程【第01章03 Keil5安装及注意事项】    请到以下网址获取下载地址：http://www.zkaifa.com/bbs/thread-12-1-1.html
*			 2.关于ITM如何使用  ，视频教程【第01章04 μCOS代码下载及环境建立】请到以下网址获取下载地址：http://www.zkaifa.com/bbs/thread-12-1-1.html
*机构：助开发网(www.zkaifa.com)
*论坛：www.zkaifa.com/bbs
*淘宝：http://shop68304486.taobao.com/
*作者：liubinkaixin
*时间：2015-05-30
*声明：当前版本仅提供核心板测试，仅提供研究学习使用，若作为商业用途出现任何错误，提供方不承担任何责任
*感谢：程序参考《正点原子@ALIENTEK》提供的源代码进行编写，在此非常感谢他们的工程师们
*/


typedef struct{
	INT8U ucMsgBuf[MAX_BUFFER_LEN];
	INT32U dwLen;
}MSG_BUF_STRUCT;

//TCP Client接收数据缓冲区
MSG_BUF_STRUCT tcp_client_recvbuf;
//TCP服务器发送数据内容
MSG_BUF_STRUCT tcp_client_sendbuf;

LB_TMRCB *TimerTcpPeriod;
LB_TMRCB *TimerTcpTimout;
LB_TMRCB *TimerSendTimout;

BOOL bSendBusy = FALSE;
BOOL bReConnFlag = FALSE;

const char get_requst[]="GET /devices/284289/datapoints HTTP/1.1\r\napi-key: UxvFGrtEFZMnfhwF6pa6zV5h9ZwA\r\nHost: api.heclouds.com\r\n\r\n";

//u8 uRemoteIp[4] = {121,40,173,197};
//u8 uRemoteIp[4] = {192,168,5,99};
//u8 uRemoteIp[4] = {192,168,11,196};
//const u8 *tcp_client_sendbuf="Explorer STM32F407 TCP Client send data\r\n";

//TCP Client 测试全局状态标记变量
//bit7:0,没有数据要发送;1,有数据要发送
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上服务器;1,连接上服务器了.
//bit4~0:保留
u8 tcp_client_flag;
u8 connflag=0;		//连接标记

//设置远端IP地址
void tcp_client_set_remoteip(void)
{
    u8 *tbuf;
		//u8 uRemoteIp[4] = {192,168,5,99};
    uart_dbg_printf((u8 *)"助开发网\n");
    tbuf=mymalloc(SRAMIN,100);	//申请内存
    if(tbuf==NULL)return;
    //前三个IP保持和DHCP得到的IP一致
    lwipdev.remoteip[0]=uRemoteIp[0];
    lwipdev.remoteip[1]=uRemoteIp[1];
    lwipdev.remoteip[2]=uRemoteIp[2];
    lwipdev.remoteip[3]=uRemoteIp[3];
    sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
    uart_dbg_printf(tbuf);

    myfree(SRAMIN,tbuf);
}

//判断发送是否空闲
BOOL IsSendBusy(void)
{
	return bSendBusy;
}

void SendTimout(void)
{
	bSendBusy = FALSE;
	DisLB_TMR(TimerSendTimout);//关闭发送超时
	printf("执行TcpDataSend超时！\n");
}

//发送数据
BOOL TcpDataSend(BYTE *buf,WORD len)
{
	int i;
	if(bSendBusy == FALSE)
	{
		printf("执行TcpDataSend\n");
		for(i = 0;i < len;i ++)
		{
			tcp_client_sendbuf.ucMsgBuf[i] = buf[i];
		}
		tcp_client_sendbuf.dwLen = len;
		bSendBusy = TRUE;
		tcp_client_flag|=1<<7;//标记要发送数据
		EnLB_TMR(TimerSendTimout);
		return TRUE;
	}

	return FALSE;
}

void key_check(void)
{
	if(0 ==GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8))
	{
		printf("key fst down!\n");
		//注意这里要减1，因为字符串最后一个'\0'也算进去了，会导致OneNet服务器不识别
		TcpDataSend((BYTE *)get_requst,sizeof(get_requst)-1);
	}
}

void TimerTcpPeroidCallBack(void)
{
	static int t = 0;
	t++;
	if(t==200)	//200ms
	{
		t = 0;
		bReConnFlag = TRUE;
	}
	if(0 == (t%100))//100ms
	{
		key_check();
	}
	//这个是lwip内核相关的，必须定时调用
	lwip_periodic_handle();
}

void TcpTimout(void)
{
	connflag=0;
	tcp_client_flag=0;
}

//TCP Client 测试
void tcp_client_test(void)
{
	struct tcp_pcb *tcppcb;  	//定义一个TCP服务器控制块
	struct ip_addr rmtipaddr;  	//远端ip地址
	u8 *tbuf;
	
		
	TimerTcpPeriod = LB_TMRCreate(1,TRUE,(TIMER_CB *)TimerTcpPeroidCallBack,(TIMER_CB *)0,(void *)0);
	

	tcp_client_set_remoteip();//先选择IP

	uart_dbg_printf((u8 *)"www.zkaifa.com\n");
	tbuf=mymalloc(SRAMIN,200);	//申请内存
	if(tbuf==NULL)return ;		//内存申请失败了,直接退出
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
	uart_dbg_printf(tbuf);
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
	uart_dbg_printf(tbuf);
	sprintf((char*)tbuf,"Remote Port:%d\n",TCP_CLIENT_PORT);//客户端端口号
	uart_dbg_printf(tbuf);

	uart_dbg_printf((u8 *)"STATUS:Disconnected\n");
	tcppcb=tcp_new();	//创建一个新的pcb
	if(tcppcb)			//创建成功
	{
			IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
			tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
	}

	TimerTcpTimout = LB_TMRCreate(2000,TRUE,(TIMER_CB *)TcpTimout,(TIMER_CB *)0,(void *)0);
	TimerSendTimout = LB_TMRCreate(1500,FALSE,(TIMER_CB *)SendTimout,(TIMER_CB *)0,(void *)0);

	while(1)
	{
		
		if(tcp_client_flag&1<<6)//是否收到数据?
		{
			printf("接收到%d字节数据:\n%s\n",tcp_client_recvbuf.dwLen,tcp_client_recvbuf.ucMsgBuf);
			//处理数据
			
			tcp_client_flag&=~(1<<6);//标记数据已经被处理了.
		}
		if(tcp_client_flag&1<<5)//是否连接上?
		{
			if(connflag==0)
			{
				uart_dbg_printf((u8 *)"STATUS:Connected   \n");//提示消息
				connflag=1;//标记连接了
			}
		}
		else if(connflag)
		{
			uart_dbg_printf((u8 *)"STATUS:Disconnected\n");
			connflag=0;	//标记连接断开了
		}
				
		//400ms检测依次是否要重连
		if(bReConnFlag == TRUE)
		{
			bReConnFlag = FALSE; 
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//未连接上,则尝试重连
			{
				ReCntLB_TMR(TimerTcpTimout);
				tcp_client_connection_close(tcppcb,0);//关闭连接
				tcppcb=tcp_new();	//创建一个新的pcb
				if(tcppcb)				//创建成功
				{
						tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
				}
			}
		}
		//定时器调度用的，放这里就行了
		LB_ExcTMR();
	}
}
//lwIP TCP连接建立后调用回调函数
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    struct tcp_client_struct *es=NULL;
    if(err==ERR_OK)
    {
        es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //申请内存
        if(es) //内存申请成功
        {
            es->state=ES_TCPCLIENT_CONNECTED;//状态为连接成功
            es->pcb=tpcb;
            es->p=NULL;
            tcp_arg(tpcb,es);        			//使用es更新tpcb的callback_arg
            tcp_recv(tpcb,tcp_client_recv);  	//初始化LwIP的tcp_recv回调功能
            tcp_err(tpcb,tcp_client_error); 	//初始化tcp_err()回调函数
            tcp_sent(tpcb,tcp_client_sent);		//初始化LwIP的tcp_sent回调功能
            tcp_poll(tpcb,tcp_client_poll,1); 	//初始化LwIP的tcp_poll回调功能
            tcp_client_flag|=1<<5; 				//标记连接到服务器了
            err=ERR_OK;
        }
        else
        {
            tcp_client_connection_close(tpcb,es);//关闭连接
            err=ERR_MEM;	//返回内存分配错误
        }
    }
    else
    {
        tcp_client_connection_close(tpcb,0);//关闭连接
    }
    return err;
}
//lwIP tcp_recv()函数的回调函数
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
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
            memset(tcp_client_recvbuf.ucMsgBuf,0,TCP_CLIENT_RX_BUFSIZE);  //数据接收缓冲区清零
            for(q=p; q!=NULL; q=q->next) //遍历完整个pbuf链表
            {
                //判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
                //的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
                if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf.ucMsgBuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//拷贝数据
                else memcpy(tcp_client_recvbuf.ucMsgBuf+data_len,q->payload,q->len);
                data_len += q->len;
                if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出
            }
						tcp_client_recvbuf.dwLen = data_len;
            tcp_client_flag|=1<<6;		//标记接收到数据了
            tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
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
//lwIP tcp_err函数的回调函数
void tcp_client_error(void *arg,err_t err)
{
    //这里我们不做任何处理
}
//lwIP tcp_poll的回调函数
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_client_struct *es;
    es=(struct tcp_client_struct*)arg;
    if(es!=NULL)  //连接处于空闲可以发送数据
    {
			ReCntLB_TMR(TimerTcpTimout);
			if(tcp_client_flag&(1<<7))	//判断是否有数据要发送
			{
				es->p=pbuf_alloc(PBUF_TRANSPORT, tcp_client_sendbuf.dwLen,PBUF_POOL);	//申请内存
				pbuf_take(es->p,(char*)tcp_client_sendbuf.ucMsgBuf,tcp_client_sendbuf.dwLen);	//将tcp_client_sentbuf[]中的数据拷贝到es->p_tx中
				tcp_client_senddata(tpcb,es);//将tcp_client_sentbuf[]里面复制给pbuf的数据发送出去
				tcp_client_flag&=~(1<<7);	//清除数据发送标志
				bSendBusy = FALSE;
				
				DisLB_TMR(TimerSendTimout);//关闭发送超时
				if(es->p)pbuf_free(es->p);	//释放内存
			}
			else if(es->state==ES_TCPCLIENT_CLOSING)
			{
				tcp_client_connection_close(tpcb,es);//关闭TCP连接
			}
			ret_err=ERR_OK;
    }
    else
    {
			tcp_abort(tpcb);//终止连接,删除pcb控制块
			ret_err=ERR_ABRT;
    }
    return ret_err;
}
//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_client_struct *es;
    LWIP_UNUSED_ARG(len);
    es=(struct tcp_client_struct*)arg;
    if(es->p)tcp_client_senddata(tpcb,es);//发送数据
    return ERR_OK;
}
//此函数用来发送数据
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
    struct pbuf *ptr;
    err_t wr_err=ERR_OK;
    while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) //将要发送的数据加入到发送缓冲队列中
    {
        ptr=es->p;
        wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
        if(wr_err==ERR_OK)
        {
            es->p=ptr->next;			//指向下一个pbuf
            if(es->p)pbuf_ref(es->p);	//pbuf的ref加一
            pbuf_free(ptr);				//释放ptr
        }
        else if(wr_err==ERR_MEM)es->p=ptr;
        tcp_output(tpcb);		//将发送缓冲队列中的数据立即发送出去
    }
}
//关闭与服务器的连接
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
    //移除回调
    tcp_abort(tpcb);//终止连接,删除pcb控制块
    tcp_arg(tpcb,NULL);
    tcp_recv(tpcb,NULL);
    tcp_sent(tpcb,NULL);
    tcp_err(tpcb,NULL);
    tcp_poll(tpcb,NULL,0);
    if(es)mem_free(es);
    tcp_client_flag&=~(1<<5);//标记连接断开了
}






















