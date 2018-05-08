一、背景和介绍
==============

1、公司/团队介绍
----------------
（1）北京叶帆易通科技有限公司（简称:叶帆科技）是中国物联网软硬中间件技术引领者，为微软BizSpark成员、阿里云物联网平台深度合作开发伙伴。专注于物联网中间件、物联网二次开发和软硬件集成技术方案研究，以成熟的.NET Micro Framework（简称.NET MF）技术为核心，用组态软件的架构和思想去构建物联网应用方案。
（2）刘洪峰（微信:yefanqiu），网名叶帆，叶帆科技创始人兼CEO，青岛普尔赛斯CTO，江苏智慧新吴高级顾问，前微软（中国）.NET MF开发团队成员。8届微软全球最有价值专家(MVP)，阿里云MVP，CSDN十大MVB。以微软.NET MF系统为核心，研发了物联网智能网关、YFIOs和YFHMI等物联网中间件软硬件平台。

2、项目介绍
-----------

**项目简介：**
智慧农业示范应用项目，最初该项目的硬件网关为Cube网关（.NET MF系统，组态式配置），其数据通过Alink上传到飞凤平台。目前嵌入式系统移植为LiteOS系统。

目前我们公司已经从事物联网领域很多年了，有一些比较成熟的物联网案例，比如和广西客户合作的物联网远程垃圾处理、污水处理，和新希望合作的物联网养殖监控，和消防领域的合作伙伴合作的物联网消防监控系统等等。

我们项目实施最大的特点就是引入了工控领域的组态概念，可以实现单芯片组态方案，便于现场实施和维护。

我们的业务模式不是最终实施项目，而是我们是和合作伙伴一起，共同在物联网领域深耕，共同去做项目。

目前我们在消防、环保、民用、军工、养殖和水处理等领域都有合作多年的行业合作伙伴，我们不仅为合作伙伴提供软件层面的技术，还提供了硬件设计生产能力，最大限度为合作伙伴赋能。

二、项目内容
============

1、方案说明
-----------
（1）功能说明
     系统采集当前环境下的温度（多个温度）、湿度、光照强度、PM2.5、氧气、氨气、二氧化碳等指标，同时可以控制养殖补光灯，喷灌开关等。
 当湿度过低的时候，会自动打开喷灌开关。当晚上来临，会自动打开补光灯。

（2）接入说明
     综合采集器通过RS485接入到网关设备，网关设备通过以太网接入阿里云物联网平台（一站式开发平台）。
     综合采集器和网关的通信协议为Modbus RTU。
     网关和阿里云物联网平台的通信协议为MQTT+Alink。

（3）阿里云物联网平台的简介
    目前包括：物联网开发套件基础版、高级版、边缘计算版。一站式开发平台，包括LinkDevelop、飞凤、飞燕平台。此外也提供了AliOS Things嵌入式操作系统。
    其Alink协议，可以对各种设备进行建模，网关和子设备灵活配置。并且阿里可以横向打通市场平台，可以让开发客户做横向扩展，比如买相关的模块和技术，或者出售相关的软件模块或产品。

2、硬件方案
-----------
硬件包含了综合采集器，最初是为养殖领域打造，有最初的3合1，升级为7合1，到最终的9合1。
网关目前采用的是野火STM32开发板，当然也可以选用我们的凌霄网关（需要进行liteOS移植）。

3、软件方案
-----------
嵌入式软件包括如下几部分：LiteOS核心、开发板驱动、STM32F4底层库、LWIP协议栈、MQTT库和Alink相关实现代码。
云端软件主要包括如下几部分：后台服务文件、AliAPI接口、图片和视图。

三、接入第三方云平台
====================
略

四、关键源代码解析
==================

1、程序文件介绍
---------------
Starup - 启动文件
user - 用户文件
drivers - 开发板BSP驱动
stm32f4xx_std_periph - STM32标准库
liteos/cmsis - LiteOS cmsis接口
liteos/kernel - LiteOS核心文件
liteos/arch - LiteOS主要文件
Lwip - Lwip协议栈
MQTT - MQTT库
Aliyun - 阿里云平台相关文件

2、程序主函数说明
-----------------
（1）内核初始化、硬件初始化、协议栈初始化
（2）创建三个任务：一个是设备状态和数据采集，一个是DHCP处理，另外一个是MQTT数据收发。

3、关键代码说明
---------------
（1）RS485相关代码做了特殊处理，因为在收发切换的时候会意外接收到一个数据。并且切换的时候最后一个数据会发送失败。
（2）LOS_MemAllocAlign函数（bestfit_little目录下）没有真正实现对齐，所以在Los_task.c文件中做了特殊处理。

4、上传数据代码详解
-------------------
（1）把从设备采集到的数据按Alink协议的格式进行打包
sprintf(param,  "{\"T1\":%3.1f,\"T2\":%3.1f,\"T3\":%3.1f,\"Tin\":%3.1f,\"H\":%3.1f,\"Lux\":%0.0f,\"CO2\":%d,\"O2\":%3.1f,\"NH3\":%d,\"PM25\":%d,\"LightSwitch\":%d,\"SpraySwitch\":%d,\"CommState\":%d}",				g_T1,g_T2,g_T3,g_Tin,g_H,g_Lux,g_CO2,g_O2,g_NH3,g_PM25,g_LightSwitch,g_SpraySwitch,g_CommState);
int msg_len = sprintf(msg_pub, ALINK_BODY_FORMAT, ++SendCount, param,ALINK_METHOD_PROP_POST);
（2）把数据推送到云端
 rc = IOT_MQTT_Publish(pclient, ALINK_TOPIC_PROP_POST, &topic_msg);

3、下发命令代码详解
-------------------
（1）执行数据处理，接收信息反馈
IOT_MQTT_Yield(pclient, 200);	
（2）订阅反馈
iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
//下行数据
if(ptopic_info->topic_len==50)
{
        DeviceState = 3;
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

	 	
五、产品调试
============

1、硬件调试
-----------
（1）RS485接口接入综合采集器
（2）以太网口插入网线

2、软件调试
-----------
（1）硬件接入后，部署上最新的程序，然后运行。
（2）把开发板USB转串口接入电脑，打开串口调试器（115200，无校验），可以看到各种运行信息。
（3）打开阿里云物联网平台，在设备运行状态中，可以看到上传的数据
（4）打开http://182.92.177.94:9990/test网页，看相关客户端页面，相关的数据是否已经正确显示。
（5）操作网页上的光照和喷淋按钮，看看相关的状态是否设置正确。

