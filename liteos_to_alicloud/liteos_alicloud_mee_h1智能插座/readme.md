一、背景和介绍
==============

1、公司/团队介绍
----------------
（1）深圳市慧视通科技股份有限公司成立于2005年，注册资金6000万元，是集研发、生产、销售、服务于一体的大型北斗/GPS专业领域综合运营服务提供商之一，高新技术企业，双软认证企业。
（2）慧视通专注于智能交通领域，以车联网、云计算、大数据处理、无线通信和北斗/GPS全球定位技术为核心，针对巡游/网约出租车、物流/两客一危、前装新能源、驾驶培训、汽车租赁、试乘试驾等行业提供符合客户需求的产品和解决方案，为客户创造价值。

2、项目介绍
-----------

**项目简介：**
智能插座项目，把传统插座接入网络，能够实现远程查看插座状态和设置电源状态。

项目利用MQTT协议接入阿里云平台，实现了远程控制插座，嵌入式系统为LiteOS系统。目前通过此项目已成功对接阿里云平台，下一阶段将CAN采集的车辆信息通过LiteOs+MQTT对接慧视通云平台和阿里云平台，使客户有更多的选择。

二、项目内容
============

1、方案说明
-----------
（1）功能说明
     设备上电连接至阿里云平台,通过云平台可以查看或设置插座状态。

（2）接入说明
     网关设备通过IO口控制继电器状态进而控制插线板电源，网关设备通过以太网接入阿里云物联网平台。
     网关和阿里云物联网平台的通信协议为MQTT+Alink。

2、硬件方案
-----------
网关目前采用的是野火STM32开发板。

3、软件方案
-----------
嵌入式软件包括如下几部分：LiteOS核心、开发板驱动、STM32F4底层库、LWIP协议栈、MQTT库和Alink相关实现代码。

LiteOS + LWIP + MQTT + ALINK <----> 阿里云


三、接入第三方云平台
====================
略

四、关键源代码解析
==================

1、程序文件介绍
---------------
starup - 启动文件
user - 用户文件
drivers - 开发板BSP驱动
stm32f4xx_std_periph - STM32标准库
liteos/cmsis - LiteOS cmsis接口
liteos/kernel - LiteOS核心文件
liteos/arch - LiteOS主要文件
lwip - Lwip协议栈
mqtt - MQTT库
alibaba - 阿里云平台相关文件

2、程序主函数说明
-----------------
（1）内核初始化、硬件初始化、协议栈初始化
（2）创建三个任务：一个是开始任务，一个是DHCP处理，另外一个是mqtt数据收发。

3、关键代码说明
---------------
（1）LwIP_DHCP_task任务，主要处理TCPIP相关业务。
（2）mqtt_client任务，主要处理阿里平台的登录和数据的收发，进而控制IO状态，是整个业务的核心。

4、上传数据代码详解
-------------------
（1）把电源状态按Alink协议的格式进行打包
		  //{"method":"thing.event.property.post","id":"2008001","params":{"PowerSwitch":1},"version":"1.0.0"}
      static int serial;
      sprintf(msg_pub, ALINK_BODY_FORMAT,ALINK_METHOD_PROP_POST, ++serial,UserSwitch);
（2）把数据推送到云端
     rc = IOT_MQTT_Publish(pclient, ALINK_TOPIC_PROP_POST, &topic_msg);

3、下发命令代码详解
-------------------
（1）执行数据处理，接收信息反馈
    /* handle the MQTT packet received from TCP or SSL connection */
    IOT_MQTT_Yield(pclient, 200);
（2）订阅反馈
    static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
    
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


	 	
五、产品调试
============

1、硬件调试
-----------
（1）以太网口插入网线

2、软件调试
-----------
（1）上电运行。
（2）把开发板USB转串口接入电脑，打开串口调试器（115200，无校验），可以看到各种运行信息。
（3）打开阿里云物联网平台，在设备运行状态中，可以看到上传的数据，在调试状态可以设置状态。
（4）阿里地址：https://account.aliyun.com/login/login.htm?qrCodeFirst=false&oauth_callback=https%3A%2F%2Fliving.aliyun.com%2F#/

