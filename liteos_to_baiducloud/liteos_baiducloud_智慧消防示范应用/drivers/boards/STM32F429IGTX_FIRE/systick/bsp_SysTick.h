#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f4xx.h"

//在滴答定时器中断服务函数中调用
void TimingDelay_Decrement(void);

// 初始化系统滴答定时器
void SysTick_Init(void);


//提供给应用程序调用
void Delay_ms(__IO u32 nTime);
#define Delay_10ms(x) Delay_ms(10*x)

#endif /* __SYSTICK_H */
