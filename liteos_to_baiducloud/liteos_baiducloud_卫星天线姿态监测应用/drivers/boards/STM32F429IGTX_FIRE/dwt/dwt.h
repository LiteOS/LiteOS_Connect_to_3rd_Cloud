#ifndef __DWT_H_
#define __DWT_H_
#include "stm32f4xx.h"
#include "los_hwi.h"
#define  DWT_CR      *(volatile uint32_t *)0xE0001000

#define  DWT_CYCCNT  *(volatile uint32_t *)0xE0001004

#define  DEM_CR      *(volatile uint32_t *)0xE000EDFC

#define  DEM_CR_TRCENA                   (1 << 24)

#define  DWT_CR_CYCCNTENA                (1 <<  0)




#define Delayms(msec)         Delayus(msec*1000)  //对于延时毫秒级的只需要定义一个宏

void DelayInit(uint32_t clk);
void Delayus(uint32_t usec);
void Delay10ms(__IO u32 nTime);
#endif
