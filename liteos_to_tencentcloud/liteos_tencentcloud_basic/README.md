# 一、背景和介绍

## 1、公司/团队介绍
[洪业](https://github.com/ianhom)，嵌入式爱好者。

## 2、项目介绍    
- 嵌入式终端通过**LiteOS**连接到**腾讯云物联网平台**。    

# 二、项目内容
## 1、方案说明
- 基于STM32F429平台，通过LiteOS + LwIP + MQTT连接到[腾讯云物联网平台](https://cloud.tencent.com/product/iothub)。STM32F429平台所采集的数据通过有线网络上传到云端，实现云端平台数据展示。

## 2、硬件方案
- [野火STM32F429挑战者开发板](https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-10310241588.32.31936ab2hZzHfP&id=545418358219)
- ARM Cortex-M4, 180MHz, 256kB RAM, 1MB Flash
- 板载ETH/WiFi

## 3、软件方案
- [LiteOS](https://github.com/LiteOS/LiteOS)
- [LwIP + MQTT](http://savannah.nongnu.org/projects/lwip/)

# 三、接入第三方云平台
- 腾讯云中物联网相关的云产品是“物联网通信”，可以通过该平台建立、管理设备在线状态，数据的上传和下发，通过消息队列等其它腾讯云产品API可以实现物联网数据的储存、处理、分析及展示等。

## 1、 创建新产品
- 登录[腾讯云平台官网](https://cloud.tencent.com/)，注册/登录账号，进入控制台，如图所示，在云产品中选择“**物联网通信**”
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/add_pro.png?raw=true)
- 点击**创建产品**
- 选择服务器**所属地区**，当前只有广州可选。
- 输入**产品名称**，该名称是一类产品的定义，例如“LiteOS_Temperature_Sensor”
- 选择**认证方式**，本例采用**密钥认证**
- 可选填入**产品描述**
- 点击**创建**即可完成新产品建立，此时将获得**productID**，该ID是终端通过MQTT访问云端的重要凭证之一。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/productID.png?raw=true)

## 2、 创建新设备
- 上述的新产品是创建一个产品类别，在该类别下需要创建具体的设备
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/add_dev.png?raw=true)
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/add_dev2.png?raw=true)
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/add_dev3.png?raw=true)
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/add_dev4.png?raw=true)
- 如上图所示完成设备的添加，此时需记录上**设备名称**，该名称也是终端通过MQTT访问云端的重要凭证之一；同时还需要记录产品**设备密钥**。

## 3、 创建消息队列
- 接下来需要为该类产品创建一个消息队列，用户数据的交互，如图所示，选择**设备上报消息**，**设备状态变化通知**，然后点击保存。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/MQ.png?raw=true)
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/MQ_rev.png?raw=true)
- 完成消息队列的创建后，即可在云产品中的**消息队列CMQ**中查看终端上传的消息。

## 4、 终端设备的配置
- 在终端平台上完成LiteOS+LwIP+MQTT的移植。
- 腾讯云平台通过[MQTT](http://mqtt.org/)接入，所以这里需要在硬件终端上实现MQTT Client，具体配置参数如下：
    - MQTT服务器地址：**iotcloud-mqtt.gz.tencentdevices.com**或**111.230.189.156**
    - ClientID：productID + 设备名称，例如：DXJQTLK47XSensor1
    - User Name：ClientID + app ID + connect ID + timestamp，例如：DXJQTLK47XSensor1;21010406;12365;1526229718
    - Password：decode(User Name + 设备密钥)+";hmacsha1",例如：b21b6c729215d2baa934878812d8947e15202618;hmacsha1
    - Publish topic：producID/设备名称/event，例如：DXJQTLK47X/Sensor1/event
    - Subscribe topic：producID/设备名称/control，例如：DXJQTLK47X/Sensor1/control
- **注意上述User Name和Password可通过工具自动生成**。

## 5、 终端设备上线和数据上传
- 完成上述配置后，即可运行终端设备，连接云端服务器，发送数据


# 四、关键源代码解析

## 1、程序文件介绍
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

## 2、程序主函数说明
（1）内核初始化、硬件初始化、协议栈初始化
（2）创建三个任务：一个是设备状态和数据采集，一个是DHCP处理，另外一个是MQTT数据收发。

## 3、关键代码说明
（1）RS485相关代码做了特殊处理，因为在收发切换的时候会意外接收到一个数据。并且切换的时候最后一个数据会发送失败。
（2）LOS_MemAllocAlign函数（bestfit_little目录下）没有真正实现对齐，所以在Los_task.c文件中做了特殊处理。

## 4、上传数据代码详解
（1）把从设备采集到的数据按Alink协议的格式进行打包
sprintf(param,  "{\"T1\":%3.1f,\"T2\":%3.1f,\"T3\":%3.1f,\"Tin\":%3.1f,\"H\":%3.1f,\"Lux\":%0.0f,\"CO2\":%d,\"O2\":%3.1f,\"NH3\":%d,\"PM25\":%d,\"LightSwitch\":%d,\"SpraySwitch\":%d,\"CommState\":%d}",				g_T1,g_T2,g_T3,g_Tin,g_H,g_Lux,g_CO2,g_O2,g_NH3,g_PM25,g_LightSwitch,g_SpraySwitch,g_CommState);
int msg_len = sprintf(msg_pub, ALINK_BODY_FORMAT, ++SendCount, param,ALINK_METHOD_PROP_POST);
（2）把数据推送到云端
 rc = IOT_MQTT_Publish(pclient, ALINK_TOPIC_PROP_POST, &topic_msg);

## 3、下发命令代码详解
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

	 	
# 五、产品调试

## 1、硬件调试
（1）RS485接口接入综合采集器
（2）以太网口插入网线

## 2、软件调试
（1）硬件接入后，部署上最新的程序，然后运行。
（2）把开发板USB转串口接入电脑，打开串口调试器（115200，无校验），可以看到各种运行信息。
（3）打开阿里云物联网平台，在设备运行状态中，可以看到上传的数据
（4）打开http://182.92.177.94:9990/test网页，看相关客户端页面，相关的数据是否已经正确显示。
（5）操作网页上的光照和喷淋按钮，看看相关的状态是否设置正确。

