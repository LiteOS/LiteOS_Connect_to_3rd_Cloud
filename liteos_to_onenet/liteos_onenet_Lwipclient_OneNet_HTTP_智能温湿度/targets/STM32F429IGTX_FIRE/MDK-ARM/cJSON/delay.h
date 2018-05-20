#ifndef __DELAY_H
#define __DELAY_H 			   
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

/*
*功能：STM32F407VET6以太网核心板测试代码，自动获取IP，并建立服务器，可通过串口1打印当前数据信息
*机构：助开发网(www.zkaifa.com)
*论坛：www.zkaifa.com/bbs
*作者：liubinkaixin
*时间：2015-05-30
*声明：当前版本仅提供核心板测试，仅提供研究学习使用，若作为商业用途出现任何错误，提供方不承担任何责任
*/

void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif





























