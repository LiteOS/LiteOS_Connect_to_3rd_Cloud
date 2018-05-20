#define _WDT_C_
#include "sys1.h"

void IwdgConfiguration(void)
{
 /* 写入0x5555,用于允许狗狗寄存器写入功能 */
 IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
 
 /* 狗狗时钟分频,40K/256=156HZ(6.4ms)*/
 IWDG_SetPrescaler(IWDG_Prescaler_256);
 
 /* 喂狗时间 640MS .注意不能大于0xfff*/
 IWDG_SetReload(100);
 
 /* 喂狗*/
 IWDG_ReloadCounter();
 
 /* 使能狗狗*/
 IWDG_Enable();
}

void WdtClr(void)
{
  IWDG_ReloadCounter();
}

/*模块结构体初始化*/
void WdtStructInit(void)
{

}

/*模块初始化*/
void WdtInit(void)
{

	WdtStructInit();
	IwdgConfiguration();

}

