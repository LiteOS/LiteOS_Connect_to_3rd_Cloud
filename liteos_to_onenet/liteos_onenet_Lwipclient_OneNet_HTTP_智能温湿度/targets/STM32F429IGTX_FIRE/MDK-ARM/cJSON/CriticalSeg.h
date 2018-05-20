/*
***********************************************************************************
*			
*功能：
*
*
*
*说明：1.
*      2.
*      3.
*
*
*By			:Liubing（开心就好）
*Contact	:371007204@qq.com
*History	:2015/1/27 9:01:07
***********************************************************************************
*/
#ifndef _CRITICALSEG_H_
#define _CRITICALSEG_H_

#include "lb_type.h"

#ifdef _CRITICALSEG_C_
#define CRITICALSEG_EXT
#else 
#define CRITICALSEG_EXT extern
#endif 

#ifdef _CRITICALSEG_C_
#endif 

#define OS_ENTER_CRITICAL  STM32_DisableIRQ()
#define OS_EXIT_CRITICAL  STM32_EnableIRQ() 

CRITICALSEG_EXT void STM32_EnableIRQ( void );
CRITICALSEG_EXT void STM32_DisableIRQ( void );


#endif
