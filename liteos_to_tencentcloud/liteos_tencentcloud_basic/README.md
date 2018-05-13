# 一、背景和介绍

## 1、公司/团队介绍
[洪业](https://github.com/ianhom)，嵌入式操作系统爱好者，小型操作系统[MOE](https://github.com/ianhom/MOE)作者;热爱物联网，对脚本语言在嵌入式开发中的应用有浓厚的兴趣。

## 2、项目介绍    
- 嵌入式终端通过**LiteOS**连接到**腾讯云物联网平台**,实现基础的连接和数据传输。    

# 二、项目内容
## 1、方案说明
- 腾讯云平台提供了丰富的云产品，其中包括了IoT方向的“物联网通信”模块，通过该模块，终端设备可以通过MQTT或CoAP接入腾讯物联网平台，实现设备的管理和数据的交互。
- 基于STM32F429平台，通过LiteOS + LwIP + MQTT连接到[腾讯云物联网平台](https://cloud.tencent.com/product/iothub)。STM32F429平台所采集的数据通过有线Ethernet+LwIP+MQTT传到云端，实现云端平台数据展示。


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
   - **MQTT服务器地址**：**iotcloud-mqtt.gz.tencentdevices.com**或**111.230.189.156**，端口号为**1883**。        
   - **ClientID**：productID + 设备名称，例如：DXJQTLK47XSensor1           
   - **User Name**：ClientID + app ID + connect ID + timestamp，例如：DXJQTLK47XSensor1;21010406;12365;1526229718     
   - **Password**：decode(User Name + 设备密钥)+";hmacsha1",例如：b21b6c729215d2baa934878812d8947e15202618;hmacsha1     
   - **Publish topic**：producID/设备名称/event，例如：DXJQTLK47X/Sensor1/event    
   - **Subscribe topic**：producID/设备名称/control，例如：DXJQTLK47X/Sensor1/control     
   
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/code.png?raw=true) 
- **注意上述User Name和Password可通过[Python工具](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/tool/UsrName_Psk.py)自动生成**。
- ![创建新产品](https://github.com/ianhom/LiteOS_Connect_to_3rd_Cloud/blob/master/liteos_to_tencentcloud/liteos_tencentcloud_basic/pic/tool.png?raw=true) 


# 四、 关键源码解析
## 1、程序文件介绍
- LiteOS内核源码：负责OS运行，提供内核服务和硬件驱动
- LwIP源码：负责实现TCP/IP通信
- MQTT源码：负责实现MQTT客户端
- Usr_Cfg.h：用以配置MQTT客户端相关参数

## 2、程序主函数说明
```c
int main(void)
{
    UINT32 uwRet;

    /*Init LiteOS kernel */
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK) {
        return LOS_NOK;
    }
    
    /* Enable LiteOS system tick interrupt */
    LOS_EnableTick();
    
    /* Init Key,LED,Uart,Eth,TCP/IP and start the MQTT client task */
    LOS_EvbSetup();
    printf("this is LiteOS lwip port \r\n");

    /* Kernel start to run */
    LOS_Start();
    for (;;);
    /* Replace the dots (...) with your own code. */
```
- 初始化LiteOS内核之后，即开始初始化硬件key，led，uart及Eth，然后即启动MQTT客户端任务。

## 3、关键代码说明
```c
void example_do_connect(mqtt_client_t *client)
{
    struct mqtt_connect_client_info_t ci;
    err_t err;
    /* Setup an empty client info structure */
    memset(&ci, 0, sizeof(ci));
    
    /* Client ID, user name, password */ 
    ci.client_id   = LOS_IOT_CLIENT_ID;
    ci.client_user = LOS_IOT_USR_NAME;
    ci.client_pass = LOS_IOT_PASSWORD;
    ci.keep_alive  = 60;

    /* MQTT server IP address */
    IP4_ADDR(&mqttServerIpAddr, 111,230,189,156);
    
    err = mqtt_client_connect(client, &mqttServerIpAddr, 1883, mqtt_connection_cb, 0, &ci);
    
    /* For now just print the result code if something goes wrong */
    if(err != ERR_OK) 
    {
        printf("mqtt_connect return %d\n", err);
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    err_t err;
    if(status == MQTT_CONNECT_ACCEPTED) 
    {
        printf("mqtt_connection_cb: Successfully connected\n");
        
        /* Setup callback for incoming publish requests */
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
        
        /* Subscribe to a topic named "subtopic" with QoS level 2, call mqtt_sub_request_cb with result */ 
        err = mqtt_subscribe(client, LOS_IOT_SUB_TOPIC, 1, mqtt_sub_request_cb, arg);
        
        if(err != ERR_OK)
        {
            printf("mqtt_subscribe return: %d\n", err);
        }
    } 
    else 
    {
        printf("mqtt_connection_cb: Disconnected, reason: %d\n", status);
        
        /* Its more nice to be connected, so try to reconnect */
        example_do_connect(client);
    }  
}

```
- `void example_do_connect(mqtt_client_t *client)`该函数负责MQTT客户端连接服务器，将在这里填入Client ID、User Name以及Password。
- `static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)`该函数作为MQTT连接的回调函数，来响应MQTT服务器连接回应。若连接成功，则开始订阅`LOS_IOT_SUB_TOPIC`主题。

## 4、上传数据代码详解
```c
void example_publish(mqtt_client_t *client, void *arg)
{
    
    const char *pstr= LOS_IOT_PUB_DATA;
    err_t err;

    u8_t qos = 1; /* 0 1 or 2, see MQTT specification */ 
    u8_t retain = 0; /* No don't retain such crappy payload... */
    
    err = mqtt_publish(client, LOS_IOT_PUB_TOPIC, pstr, strlen(pstr), qos, retain, mqtt_pub_request_cb, arg);
    
    if(err != ERR_OK)
    {
        printf("Publish err: %d.\r\n", err);
    }
    else
    {
        printf("Publish Success.\r\n");
    }
}

/* Called when publish is complete either with sucess or failure */
void mqtt_pub_request_cb(void *arg, err_t result)
{
    if(result != ERR_OK)
    {
        printf("Publish result: %d\n", result);
    }
}
```
- `void example_publish(mqtt_client_t *client, void *arg)`函数负责数据publish到`LOS_IOT_PUB_TOPIC`主题中。
- `void mqtt_pub_request_cb(void *arg, err_t result)`函数作为publish结果的回调函数，对publish的结果进行响应。
## 5、 下发命令代码详解
```c
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    printf("Incoming publish at topic \" %s \" with total length %u\r\n", topic, (unsigned int)tot_len);
}
```
-`mqtt_subscribe(client, LOS_IOT_SUB_TOPIC, 1, mqtt_sub_request_cb, arg);`该函数向服务器订阅了`LOS_IOT_SUB_TOPIC`主题。
-`static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)`该回调函数在收到下行报文后答应报文信息。

# 五、 产品调试
- 运行终端设备，连接云端服务器，发送数据。
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
