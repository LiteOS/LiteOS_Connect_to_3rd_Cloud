# 一、背景和介绍

## 1、公司/团队介绍
[洪业](https://github.com/ianhom)，嵌入式爱好者。

## 2、项目介绍    
- 嵌入式终端通过**LiteOS**连接到**腾讯云物联网平台**,实现基础的连接和数据传输。    

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
   - **MQTT服务器地址**：**iotcloud-mqtt.gz.tencentdevices.com**或**111.230.189.156**        
   - **ClientID**：productID + 设备名称，例如：DXJQTLK47XSensor1           
   - **User Name**：ClientID + app ID + connect ID + timestamp，例如：DXJQTLK47XSensor1;21010406;12365;1526229718     
   - **Password**：decode(User Name + 设备密钥)+";hmacsha1",例如：b21b6c729215d2baa934878812d8947e15202618;hmacsha1     
   - **Publish topic**：producID/设备名称/event，例如：DXJQTLK47X/Sensor1/event    
   - **Subscribe topic**：producID/设备名称/control，例如：DXJQTLK47X/Sensor1/control     
   
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/code.png?raw=true) 
- **注意上述User Name和Password可通过[工具]()自动生成**。

## 5、 终端设备上线和数据上传
- 完成上述配置后，即可运行终端设备，连接云端服务器，发送数据。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/online.png?raw=true) 
- 可以在设备页面检测设备是否上线。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/msg_rcv1.png?raw=true)
- 可以通过消息队列来查看终端上传的数据。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/msg_rcv2.png?raw=true) 
- 上图为设备上线后的状态报告。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/msg_rcv3.png?raw=true) 
- 上图为终端发送的数据，payload部分为base64加密后的格式。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/msg_rcv4.png?raw=true)
- 通过base64解密即可得到数据原文。

# 四、 总结
- 得益于LiteOS完善的软件和活跃的开发者氛围，IoT方向的移植工作可以很顺利的完成，感谢@[夏晓文](https://github.com/xiaowenxia)关于LwIP+MQTT的移植分享。
- 该项目实现了最基本的云平台对接和数据上传，期待其他小伙伴们在该平台上的精彩作品:smile:
- 腾讯云的消息队列提供了丰富的[API](https://cloud.tencent.com/document/api/406/5853)，可以在web，APP端直接获取物联网上传的数据和下发控制报文，此项工具将持续完善，为大家呈现更完整有趣的应用。
- LiteOS日益完善、强大，期待LiteOS全面解锁IoT！
