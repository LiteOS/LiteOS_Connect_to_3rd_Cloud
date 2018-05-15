# 【寻找IoT达人】LiteOS对接云平台作品展示，卫星天线姿态监测应用


一、背景和介绍
==============

1、公司/团队介绍
----------------
业余开发爱好者，目前专注于工业物联网相关技术。  


2、项目介绍
-----------

**项目简介：**
科技日新月异的发展，信息化数字化的不断推广和应用，使我们的生活方式发生了巨大变化，也给传统技术带来了新的变革Internet信号的传输国际通信电视信号的传输等利用同步卫星进行通信是重要的技术手段同步卫星在轨运行年久之后，相比一开始的姿态会发生变化，严重时会偏移运行轨道，地面的卫星天线仍然指向最初的预定位置，导致卫星通信的中断另一方面，地面大口径卫星天线受到常年风力及重力的影响，也会偏离预定指向，造成天线的方向系数与效率的乘积明显下降，信息交流传递产生中断另一方面，卫星天线控制室，往往布设在高山或者高楼顶层，环境恶劣，电子辐射较大，影响值机人员的身体健康针对该问题，设计了一种具有低功耗支持远程数据传输实时反馈卫星天线姿态变化，并能进行远程监测和卫星通信天线角度方位控制的应用系统。在接收数字信号时，该系统能够满足在较短的时间内使天线准确对星，保证信号接收的快速可靠。


二、项目内容
============

1、方案说明
-----------
本项目使用STM32F429IGT6开发板，通过MPU6050测量姿态数据，使用MQTT协议把姿态数据上传到百度云。 
![avatar](http://developer.huawei.com/ict/forum/data/attachment/forum/201805/13/110136x3435qhhseqtmofq.jpg)
百度IOT HUB设置
![avatar](http://developer.huawei.com/ict/forum/data/attachment/forum/201805/13/110651z0svntbnjmlnc7j9.png)
![avatar](http://developer.huawei.com/ict/forum/data/attachment/forum/201805/13/110510aeejxyoh3hgexu99.png)
网络数据抓包测试
![avatar](http://developer.huawei.com/ict/forum/data/attachment/forum/201805/13/104730ko0a0iib0geyei00.png)
用MQTT.fx测试MQTT协议，订阅姿态数据
![avatar](http://developer.huawei.com/ict/forum/data/attachment/forum/201805/13/104711ialtagrvt9oklzlt.png)
![avatar](http://developer.huawei.com/ict/forum/data/attachment/forum/201805/13/104719mlekduwv9vwsnwn9.png)




