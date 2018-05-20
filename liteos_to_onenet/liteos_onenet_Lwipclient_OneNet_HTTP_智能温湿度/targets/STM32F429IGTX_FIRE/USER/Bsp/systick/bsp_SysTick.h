#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f4xx.h"
#include "los_task.h"

//在滴答定时器中断服务函数中调用
void TimingDelay_Decrement(void);

// 初始化系统滴答定时器
void SysTick_Init(void);

void Delay_us(__IO u32 nTime);
//提供给应用程序调用
void Delay_ms(__IO u32 nTime);
//void SysTick_Init1();
#define Delay_10ms(x) Delay_ms(10*x)

#endif /* __SYSTICK_H */
