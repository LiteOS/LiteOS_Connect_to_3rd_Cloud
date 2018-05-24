
一、背景和介绍
==============

1、公司/团队介绍
----------------
（1）厦门四信通信科技有限公司，福建省著名商标，福建省科技创新小巨人领先企业，智慧电力、智慧城市、智慧水利、智慧地灾等行业解决方案提供商。专注于LoRa、NB-IoT、ZigBee、2.5G、3G、LTE等无线通讯模组和终端产品，物联网测控终端及系统解决方案的研发、生产、销售及服务，为行业用户，系统集成商，运营商提供有竞争力的产品、技术、方案和服务。聚焦于IoT，围绕着行业客户，提供开放的合作理念，并致力于让万物更加智慧的愿景。

（2）本次参赛团队是由四信通信研发三部的小伙伴自发组成。(原本报名的时候，我们是叫IoT小分队。最近其他同事跟我吐槽说咱整个公司都是IoT，你不能把好名字给占了。好吧，所以我们现在只好叫研发三部了，一个很淡淡的名字。)
我们主要负责 NB-IoT、LoRa、ZigBee 等无线模组和终端产品解决方案。作为阿里IoT智慧城市合作伙伴，ICA联盟重要成员，我们希望通过此次比赛为下一步智慧城市消防方案做产品铺垫。同时希望借此机会熟悉国产优秀的操作系统 LiteOS，了解后续项目与 LiteOS 的合作可能。

2、项目介绍
-----------

**项目简介：**

（1）行业现状  

传统的独立式火灾报警设备/系统，只能够起到在火灾发生时警示现场人员疏散的作用。 消防及行政监管及消防系统运行服务商无法第一时间掌握消防系统的真实运营率和设施完好率(Eg. 故障、电池电量低、违规拆除)。并且当发生火情的时候，完全靠现场人员进行报警，无法让消防部门及其他相关救援力量(物业、微型消防站、社区警务室等)在第一时间内获取告警通知和准确火灾位置， 大大延误了出警和初起火灾扑救的宝贵时间，给火灾的扑救带来极大的阻力。  

所以基于无线通讯技术的独立式智能火灾报警系统在综合了多年来企业，机关，消防部门的各种不利，各种弊端应运而生。

（2）应用场景

![](http://7xkqvo.com1.z0.glb.clouddn.com/liteos_game_sdm200_dg.jpg)

此前四信已经和霍尼韦尔等多家企业合作过 LoRa 感烟报警器，并取得公安部认证，在厦门等地的多个项目中成功应用。随着 NB-IoT 网络的逐步覆盖，我们希望同时提供 NB-IoT 的消防产品以满足客户多样性的需求。

此次参赛作品是 四信 NB-IoT 独立感烟火灾报警器(对接阿里云)的产品原型，为后续其他消防类产品做技术预研铺垫。

（3）项目特点

- 项目代码兼容资源受限设备，便于原型往实际产品的转化

原型代码开发上避免采用 lwip 协议栈，为后续往资源受限设备上迁移做好了准备。这个决定让我们的工作量成倍地加大，但作为开源项目，我们觉得就应该给其他开发者提供一些不同的思路。

通常的做法是把串口操作封装到lwip的链路层ppp协议里，在CPU一侧来实现TCP/IP协议栈。但我们坚持采用模块AT指令来实现数据操作，把协议栈的工作继续留在了模组那一侧。原本已经实现了CoAP的数据传输。但阿里云传输协议需要使用 DTLS 加密方式，这个嵌入式加密库只有 socket 接口的实现方式。虽然在已有的DTU产品已经研究过此种方式，但 socket 接口不够标准，适配工作量比较大。后来有幸找到了AliOS的SAL（Socket抽象层）相关代码，socket 实现方式基本和标准接口接近。我们移植了SAL模块，并解决了UDP等部分的疑似BUG（我们还会和AliOS的朋友再沟通确认），顺利地完成了非lwip的实现方案。

- 采用 NB-IoT + CoAP协议 通讯方式

由于采用 BC95 这款模块，它的功能不像以太网等硬件那么方便，模块域名解析功能受限、仅支持UDP协议等，因此我们在 SNTP 协议等一些场景做了相应处理，或许能给后面其他使用这类模块的朋友一些参考。

- 阿里云物联网套件高级版首批接入

阿里云物联网套件高级版，在18年4月刚刚发布，在原本基础版提供的通道能力上扩展了设备全生命周期管理能力，包括设备模型定义、在线调试、原始数据存储、设备数据查询、设备数据推送等能力，让使用者无需考虑设备的数据格式和存储等问题。

在目前平台调试手段有限的情况，花了较多精力与阿里伙伴调试确认，最终搞定了物模型转化为Alink的一些易错点。本产品原型的Alink格式可给后续其他朋友提供一些参考。

（4）项目可行性分析

产品原型实现了一款烟感产品必要的 烟雾报警、防拆报警、电池电量采集、心跳上报周期可设 等功能，并且实现了阿里云IoT平台高级版的协议接入，可便利地集成到阿里云智慧城市项目中。

四信自研的 NB-IoT 烟感产品 F-SDM200 已完成内部硬件设计，基于目前原型已实现的功能，接下去主要在低功耗CPU上再调试下休眠业务，就可以完成实际产品功能。

另外有一点关于NB的下行IP限制问题，要重点说明下。由于电信NB卡如果没有定向到特定服务器，下行会被限制，因此我们这边找电信做了特殊处理，目前将该测试设备用到的IP进行了报备设置。预计后续大家要对接阿里云的话，估计只能等联通推出NB卡了。(因为现在阿里投资了联通)

二、项目内容
============

1、方案说明
-----------
（1）功能说明。

1.烟雾报警时可发出声光警报信号，并在平台上报警。
2.支持被拆卸时报警。
3.电池电压不足时有声光提示功能。
4.心跳上报周期可设，便于终端用户结合实际场景做定制调整。

（2）阿里云物联网平台对接说明。

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_datagram.png)

- 设备端NB-IoT模块中集成阿里云 IoT SDK，厂商在IoT套件控制台申请设备证书（ProductKey/DeviceName/DeviceSecret）并烧录到设备中；
- NB-IoT设备通过运营商的蜂窝网络进行入网，可能需要联系当地运营商，确保设备所属地区已经覆盖NB网络，并已具备NB-IoT入网能力；
- 设备入网成功后，NB设备产生的流量数据及产生的费用数据，将由运营商的M2M平台管理，此部分平台能力由运营商提供；
- 设备开发者可通过 CoAP/UDP 协议，将设备采集的实时数据上报到阿里云IoT套件，借助IoT套件实现海量亿级设备的安全连接和数据管理能力，并可通过规则引擎，与阿里云的各类大数据产品、云数据库和报表系统打通，快速实现从连接到智能的跨越；
- IoT套件提供相关的数据开放接口和消息推送服务，可将数据转发到业务服务器中，实现设备资产与实际应用的快速集成。

（3）阿里云物联网平台的简介

阿里云物联网平台目前包括：物联网开发套件基础版、高级版、边缘计算版。一站式开发平台，包括LinkDevelop、飞凤、飞燕平台。此外也提供了AliOS Things嵌入式操作系统。

其Alink协议，可以对各种设备进行建模，网关和子设备灵活配置。并且阿里可以横向打通市场平台，可以让开发客户做横向扩展，比如买相关的模块和技术，或者出售相关的软件模块或产品。

2、硬件方案
-----------
硬件构成方案为：感烟探测传感器 + STM32F429开发板 + NB-IoT模块。

3、软件方案
-----------
软件包括如下几部分：LiteOS核心、开发板驱动、STM32F4底层库、CoAP库和Alink相关实现代码。

三、阿里IoT平台接入
====================

1、账号注册
-----------------
阿里云平台的注册比较简单，使用淘宝、支付宝等账号即可。开通之前先看看阿里云怎么[收费](https://help.aliyun.com/document_detail/55733.html)。

```
100万条消息收费3.6元

每月赠送100万消息数，从当月1号凌晨开始赠送，不累计到下月。每天结算统计消息数，对累计超出100万条的消息数进行计费，未超出部分免费。
```

这就相当于免费了，大胆开通之。以aliyun账号直接进入[IoT控制台](http://iot.console.aliyun.com/)，如果还没有开通阿里云物联网套件服务，则需要申请开通。一定要记得先实名认证再开，否则无法开通。

2、产品创建
-----------------
阿里云刚在4月完成了物联网套件高级版。意味着阿里云IoT不再做通道，而是按照定义格式对数据做存储解析处理。这次我们玩的就是高级版。

```
物联网套件发布高级版，在基础版提供的通道能力进行扩展，让平台具备更加完整的设备全生命周期管理能力，包括设备模型定义、在线调试、原始数据存储、设备数据查询、设备数据推送等能力，开发者无需考虑设备的数据格式和存储等问题，进一步降低设备智能化周期和成本，让开发者可以更聚焦于垂直业务系统的搭建，快速实现智能转型。
```

在产品创建时，阿里云提供了多个功能模版，我使用了烟感模版。官方模版比较简单，只有1个属性和1个事件。我再定义相对完备的烟雾报警器。

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_fun_def.png)

高级版使用物模型来定义产品，我这边创建的感烟报警器，其物模型描述如下

```
                {
                    "identifier": "SmokeSensorState",
                    "dataType": {
                        "specs": {
                            "0": "正常",
                            "1": "检测到烟雾"
                        },
                        "type": "enum"
                    },
                    "name": "烟雾检测状态"
                },
                {
                    "identifier": "RemoveState",
                    "dataType": {
                        "specs": {
                            "0": "正常",
                            "1": "拆卸报警"
                        },
                        "type": "bool"
                    },
                    "name": "拆卸报警状态"
                },
                {
                    "identifier": "BatteryPercentage",
                    "dataType": {
                        "specs": {
                            "unit": "%",
                            "min": "0",
                            "unitName": "百分比",
                            "max": "100"
                        },
                        "type": "double"
                    },
                    "name": "电池电量"
                },
                {
                    "identifier": "Period",
                    "dataType": {
                        "specs": {
                            "unit": "″",
                            "min": "10",
                            "unitName": "秒",
                            "max": "79800"
                        },
                        "type": "int"
                    },
                    "name": "心跳上报周期"
                }
```

3、设备管理
-----------------
产品定义好，还需要添加具体的设备。

在 管理控制台->设备管理->添加设备，即可根据定义好的产品来创建设备。设备添加好之后，即可获得设备三元组：

```
ProductKey：***YourProductKey***
DeviceName：device-test
DeviceSecret：***YourDevcieSecret***
```

4、新增数据流
-------------
测试首要是实现烟感设备的属性上报，根据定义好的物模型，得到如下 Alink Json 数据格式：

```
{
    "id":"123",
    "version":"1.0",
    "params":{
        "SmokeSensorState":{
            "value":0,
            "time":1526051254000
        },
        "RemoveState":{
            "value":0,
            "time":1526051254000
        },
        "BatteryPercentage":{
            "value":99,
            "time":1526051254000
        },
        "Period":{
            "value":10,
            "time":1526051254000
        }
    },
    "method":"thing.event.property.post"
}
``` 

5、SDK模拟测试
-----------
阿里提供了IoT-SDK，方便大家测试验证。

SDK下载：
https://help.aliyun.com/document_detail/42648.html

SDK github：
https://github.com/aliyun/iotkit-embedded

我们主要使用 CoAP example: \sample\coap\coap-example.c。在这个例程上，做部分修改。

6、查看数据
-----------
SDK 改完之后，编译运行，顺利的话就能在 管理控制台->设备管理->运行状态 中看到设备上报的数据。

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_report.png)

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_report_bat.png) 

注意：格外注意的是，由于阿里云高级版才推出不久，因此在 产品管理->在线调试 是看不到任何数据的。设备状态一直都显示“未激活”。

目前依赖于阿里云的伙伴人工反馈确认，效率不高。这次调试时，阿里伙伴在深夜十一点多依旧在系统里给我答复，在此表示感谢。阿里云下一步推出日志功能，相信大家调试会更加方便了。

四、关键源代码解析
==================

1、程序文件介绍
---------------
arch - 芯片体系文件  
drivers - 开发板BSP驱动   
components - coap、mbedtls、socket抽象层等库文件  
kernel - LiteOS内核  
targets - 开发板硬件  

2、程序主函数说明
-----------------
（1）内核初始化、硬件初始化、协议栈初始化  
（2）创建子任务分别为烟感设备的硬件驱动和CoAP网络连接交互  

3、关键代码说明
---------------
整个移植过程中有几个核心点要注意。

（1）CoAP发送链表

阿里SDK与LiteOS的List接口有些许不同，移植时要小心对比，否则会出错。

（2）NTP时间同步

BC95无法直接解析阿里云NTP服务器域名，若再移植DNS协议，又会涉及运营商那边对 DNS IP 限制。因此目前代码中先人工获取了域名所对应的IP，直接跳过域名解析步骤。

（3）TLS导致的栈溢出

TLS的证书比较大，这部分内存要开大一点。

4、上传数据代码详解
-------------------

这个函数负责感烟报警器的设备属性 CoAP 上报，函数中先调用 iotx_propreport_payload_sdm200() 封装Alink Json，再调用 iotx_propreport_topic() 封装好 topic，之后使用 IOT_CoAP_SendMessage() 来发出协议报文。

```
/* report prop */
static int iotx_coap_report_prop(iotx_coap_context_t *p_context)
{
    int                     ret;
    char                    topic_name[IOTX_URI_MAX_LEN + 1];
    iotx_message_t          message;
    iotx_coap_t            *p_iotx_coap = (iotx_coap_t *)p_context;
    CoAPContext            *p_coap_ctx = NULL;

    if (NULL == p_iotx_coap) {
        log_err("Invalid param: p_context is NULL");
        return FAIL_RETURN;
    }

    log_debug("DeviceProp Report: started in CoAP");
    p_coap_ctx = (CoAPContext *)p_iotx_coap->p_coap_ctx;

    /* 1,generate json data */
    char *msg = HAL_Malloc(REPORT_PAYLOAD_LEN);
    if (NULL == msg) {
        log_err("allocate mem failed");
        return FAIL_RETURN;
    }

    iotx_propreport_payload_sdm200(msg, 0, 0, 99, 10);
    
    log_debug("Dev Report: json data = '%s'", msg);

    memset(&message, 0, sizeof(iotx_message_t));

    message.p_payload = (unsigned char *)msg;
    message.payload_len = (unsigned short)strlen(msg);
    message.resp_callback = iotx_coap_mid_rsphdl;
    message.msg_type = IOTX_MESSAGE_NON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;

    /* 2,generate topic name */
    ret = iotx_propreport_topic(topic_name,
                               "/topic",
                               p_iotx_coap->p_devinfo->product_key,
                               p_iotx_coap->p_devinfo->device_name);

    log_debug("Dev Report: topic name = '%s'", topic_name);

    if (ret < 0) {
        log_err("generate topic name of info failed");
        HAL_Free(msg);
        return FAIL_RETURN;
    }

    if (IOTX_SUCCESS != (ret = IOT_CoAP_SendMessage(p_context, topic_name, &message))) {
        log_err("send CoAP msg failed, ret = %d", ret);
        HAL_Free(msg);
        return FAIL_RETURN;
    }
    HAL_Free(msg);
    log_debug("Dev Report: IOT_CoAP_SendMessage() = %d", ret);

    ret = CoAPMessage_recv(p_coap_ctx, CONFIG_COAP_AUTH_TIMEOUT, 1);
    log_debug("Dev Report: finished, ret = CoAPMessage_recv() = %d", ret);

    return SUCCESS_RETURN;
}
```

五、产品调试
============

1、硬件调试
-----------
  
（1）实物照片

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_hw.png)

（2）接线说明

感烟传感器的连接说明：

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_line_F8L10SA.jpg)

NB-IoT模组的连接说明：

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_line_NB.jpg)

（3）烟感触发测试

如下图所示，按住火警报警测试按键，烟感会发出报警鸣叫，此时则认为是有火灾发生。若释放火警报警测试按键，则代表火警消除，此时蜂鸣器停止报警鸣叫。

火警状态发送变化（火警报警——>火警消除 或者 火警消除——>火警报警），会立即上报当前变化状态。

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_fire.png)

（4）防拆开关测试

如下图所示，防拆开关按住则代表正常状态，设备没有被拆下。若防拆开关弹开，则代表设备被拆除。正常情况下，防拆开关处于按住状态。
防拆开关状态发生变化，会立即上报当前变化状态。

![](http://7xkqvo.com1.z0.glb.clouddn.com/aliyun_iot_coap_sdm200_remove.png)


