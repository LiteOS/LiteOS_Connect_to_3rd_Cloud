一、背景和介绍
==============

1、公司/团队介绍
----------------
武汉拓宝科技股份有限公司由富有创新精神的留美博士和企业家创立，团队有成功的创业经验和丰富的高科技公司管理经验，并在无线通信、微波射频、芯片设计和软件开发方面有业内领先的产品和技术研发能力，研发团队半数以上具有硕士学位，包括博士和高级职称多人，公司拥有包括多项发明专利在内的数十项知识产权。
________________
公司致力于领先的物联网产品和系统解决方案
解决方案：
> * [无线智慧消防解决方案](http://www.turboes.com/h-col-137.html)
> * [无线智慧停车解决方案](http://www.turboes.com/h-col-136.html)

主营产品：
> * [NB-IoT 物联网终端](http://www.turboes.com/h-col-102.html)
> * [LoRa 物联网终端](http://www.turboes.com/h-col-121.html)
> * [LoRa 无线网关和模组](http://www.turboes.com/h-col-115.html)
> * [微波雷达传感器](http://www.turboes.com/h-col-125.html)

________________
具体详情请访问公司官方网站：[武汉拓宝科技股份有限公司](http://www.turboes.com/)
微信公众号：拓宝科技

2、项目介绍
-----------

**项目简介：**
> 拓宝科技基于智慧消防系统解决方案开发了终端报警探测器、无线网关、云平台、应用服务器几款产品并已经成功应用于多个场景。本次的智慧消防示范应用项目使用了华为官方提供的F429主板来模拟我们的终端报警探测器，探测器具有连接云平台，向云平台发送报警数据和接收来自云平台下发的命令和参数的功能，本次项目连接的是第三方百度云平台(物接入IoTHub)，采用了MQTT客户端来模拟应用服务器接收第三方云平台的数据，并下发控制命令和参数给百度云，本次项目仅作为我们智慧消防系统解决方案的初步演示作用。

> 在智慧消防领域中，目前市面使用的大多数产品烟雾探测报警器、手动报警按钮、可燃气体探测器等都不具备联网功能，当居所处发生火灾或紧急险情时只能本地报警（比如在深夜入睡后），救援人员无法快速发现险情，等到发现时已无法及时救援，最终晾成悲剧。近年来随着物联网的火热市面上出现了一些具备联网功能的智能报警器，当发生险情时可通过无线或有线通信的方式将报警信息及时告知救援人员，从而减少悲剧的发生，不过该类型的产品无线通信距离短、功耗较高、布线成本太高、更换麻烦，我们本次产品设计无线智慧消防产品采用NB-IOT，LoRa等无线通信技术具有传输距离长、大面积组网、休眠功耗低（uA级）、安装和拆卸方便的特点，适用于智能家居、大型社区、办公场、公共场所所等大型建筑物，未来发展前景广阔。


二、项目内容
============

1、方案说明
-----------
（1）功能说明
> * 本次项目采用的是F429开发板模拟我们的终端报警探测器，实现了终端连接百度云的物接入IoT Hub平台，模拟终端向平台发送报警数据，发送连接保持的心跳数据，接收从云平台下发的控制报警停止的命令
> * 通过按下按键K1来开启/断开终端与云平台的连接，通过按下按键K2来开始/停止发送报警信息到平台，当设备处于报警状态时，可通过MQTT客户端模拟应用服务器给云平台下发停止发送报警的命令
> * 终端处于保持连接状态时K1上方绿灯亮起，处于断开连接状态时K1上方的红灯亮起，终端处于发送报警信息的状态，终端每次发送报警数据时F429核心板上面的小led（蓝色）闪烁一次，每隔8秒左右发送一次报警数据，可通过按键K2或云平台下发控制命令取消报警
> * 终端与云服务器通信过程可通过板载的串口1输出打印信息进行查看

（2）接入说明
> * 第三方云平台：百度云 —— 物接入IoT Hub
> * 接入协议：TCP+MQTT
> * 接入方式：Ethernet(Lwip)
> * 功能架构：数据流向+应用功能
数据流向：终端(F429模拟)<---->云平台(百度云)<---->应用平台(MQTT客户端模拟)
应用功能：
1.终端断开与连接云平台
2.终端发送报警数据到云平台
3.终端从云平台接收停止报警命令

（3）物接入IoT Hub简介
> 物接入IoT Hub 是全托管的云服务，通过主流的物联网协议（如MQTT）通讯，可以在智能设备与云端之间建立安全的双向连接，快速实现物联网项目。
支持亿级并发连接和消息数，建立海量设备与云端安全可靠的双向连接，无缝对接天工平台和百度云的各项产品和服务。

2、硬件方案
-----------
本次项目硬件采用了华为官方提供的F429开发板，通过板载按键模拟报警输入信号，输出使用了板载led，串口，和RJ45网口。

3、软件方案
-----------
> * 软件整体架构：驱动层+系统层+网络层+应用层
驱动层：stm32f4官方驱动库文件+板载外设驱动文件(LED/KEY/UART/ETH/TIMER)
系统层：LiteOS系统文件
网络层：传输层(Lwip)文件+应用层(MQTT)文件，分为网络初始化、网络连接、网络断开、网络状态，发送网络数据、接收网络数据几个功能模块
应用层：智慧消防功能演示文件，主要分为连接应用服务器、断开应用服务器、发送报警数据到应用服务器、从应用服务器接收控制命令几个模块

三、接入第三方云平台
====================
* [百度云 —— 物接入IoT Hub 详情](https://cloud.baidu.com/doc/IOT/ProductDescription.html)

四、关键源代码解析
==================

1、程序文件介绍
---------------
```
arch        arm的cortex架构相关的文件
drivers     芯片相关驱动文件，板载相关外设驱动文件
kernel      LiteOS系统相关核心文件
lwip        lwip协议栈相关核心文件
mqtt        MQTT应用协议相关核心文件
baidu       用户应用相关代码
targets     目标板相关的示例工程
```

2、程序主函数说明
-----------------
（1）. 内核初始化、硬件初始化、协议栈初始化
（2）. 创建二个任务：按键处理任务，Lwip协议栈轮询处理任务
（3）. 通过按键处理任务创建4个任务：接收应用数据任务，发布主题任务，保持网络连接任务，订阅主题任务


3、关键代码说明
---------------

> （1）按键处理任务，按下K1时，终端连接MQTT服务器，连接成功绿灯亮起，连接失败红灯亮起；处于连接状态时再次按下K1，断开终端与MQTT服务器的连接,绿灯变红灯；按下K2时，终端开始每隔8秒发送报警数据给MQTT服务器，核心板上面的蓝色led闪烁一次，再次按下K2，停止发送报警数据，在处于报警状态下时可通过MQTT客户端发送控制指令给云平台，云平台将控制指令下发给终端，终端停止报警
```
VOID key_task(VOID)
{
	while(1)
	{
		Key_RefreshState(&Key1);
		Key_RefreshState(&Key2);
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
```
> （2）轮询处理lwip协议栈相关的任务，包括了协议栈的初始化，发送数据，接收数据

```
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
```

> （3）MQTT连接的目的IP和PORT

```
#define DEST_IP_ADDR0               163
#define DEST_IP_ADDR1               177
#define DEST_IP_ADDR2               150
#define DEST_IP_ADDR3                11
#define DEST_PORT                  1883
```

> （4）连接MQTT服务器的配置参数

```
#define MQTT_CLIENT_ID						"tbs_n110_001"
#define KEEPALIVE_INTERVAL 					60
#define MQTT_CLIENT_USER_NAME				"tbs_n110/tbs_n110_001"
#define MQTT_CLIENT_PASSWORD				"nu/005K89vIWmbqmP9+V4h7k2GeofYtvZF3o9JT/X8o="

#define MQTT_CLIENT_SUBTOPIC				"topic02"
#define MQTT_CLIENT_PUBTOPIC				"topic01"
```

> （5）连接MQTT服务器

```
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
```

4、上传数据代码详解
-------------------
> * 发送主题消息给MQTT服务器

```
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
```

3、下发命令代码详解
-------------------
> * 从MQTT服务器接收下行数据

```
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
```


	 	
五、产品调试
============

1、硬件调试
-----------
> * 电源适配器插入F29开发板的电源接口
* 串口线连接F429开发板的串口1和PC端(115200/8/1)
* 网线一端插入F429开发板的网口，一端接入路由器网络，动态获取IP，保证路由器联网正常

2、软件调试
-----------
> * 硬件接入后，部署上最新的程序，然后运行
* 打开PC机串口调试助手(115200/8/1/无校验)，查看开发板运行信息
* 在PC端打开MQTT客户端软件，连接MQTT服务器（云平台），订阅主题topic01，开发板会向topic01主题发布消息；向主题topic02发布下行控制命令，开发板会收到topic02的消息
* 按下按键K1，开发板接入云平台，接入成功亮绿灯，失败亮红灯，连接失败可再次按下K1尝试重新接入；接入成功后按下K2按键，开发板向主题topic01发送报警消息，此时通过MQTT客户端可收到云平台推送的报警信息；通过MQTT客户端向主题topic02发布“close the alarm”消息，停止开发板发送报警消息，也可以直接再次按下K2停止报警；测试完成，再次按下K1，断开云平台与开发板的连接

