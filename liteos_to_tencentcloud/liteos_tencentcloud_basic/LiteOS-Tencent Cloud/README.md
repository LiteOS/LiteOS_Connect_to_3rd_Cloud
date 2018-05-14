## 简介
本项目使用华为物联网操作系统Huawei LiteOS进行软件开发，暂时目标是移植最新的lwip协议栈2.0，然后运行httpserver。
## Huawei LiteOS基础内核
华为物联网操作系统Huawei LiteOS是华为面向物联网领域开发的一个基于实时内核的轻量级操作系统。本项目属于华为物联网操作系统，目前仅开源基础内核，同时适配了STM32F412/F429/L476及GD32F190/F450开发板，后续会开放其他特性同时支持其他类型开发板。现有代码支持任务调度，内存管理，中断机制，队列管理，事件管理，IPC机制，时间管理，软定时器以及双向链表等常用数据结构。

### 开发板类型
* GD32F207C-EVAL

## LiteOS 系统初始化流程
![](./doc/LiteOS_System_initialization.png)

1. liteos
2. mqtt server
3. mqtt web client
4. mqtt electron desktop app
5. weapp 微信小程序

## 开源协议
* 遵循MIT开源许可协议
