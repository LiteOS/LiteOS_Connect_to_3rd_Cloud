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
*History	:2015/2/27 8:39:52
***********************************************************************************
*/
#ifndef _WDT_H_
#define _WDT_H_

#include "lb_type.h"

#ifdef _WDT_C_
#define WDT_EXT
#else 
#define WDT_EXT extern
#endif 

#ifdef _WDT_C_
#endif 

typedef struct
{
    INT8U status;
}WdtStruct;

WDT_EXT WdtStruct WdtData;

WDT_EXT void WdtInit(void);
WDT_EXT void WdtClr(void);
#endif
