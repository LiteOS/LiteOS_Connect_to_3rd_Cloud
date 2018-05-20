#ifndef _LB_TIMER_H_
#define _LB_TIMER_H_

#include "lb_type.h"
#include "CriticalSeg.h"
#ifdef _LB_TIMER_C_
#define LB_TIMER_EXT
#else
#define LB_TIMER_EXT extern
#endif

#ifndef LB_TRUE
	#define LB_TRUE	1
#endif

#ifndef LB_FALSE
	#define LB_FALSE	0
#endif

#ifndef LB_NULL
	#define LB_NULL (void *)0	   /*空指针*/
#endif

#define XRAM 			 /*外部RAM定义*/

#define LB_MAX_TIMER	20	   //定时器最大个数

typedef void *TIMER_CB(void *msg);	 	//回调函数定义
typedef struct LB_TMRCB LB_TMRCB;	//定时器控制块结构体类型定义

//定时器控制块结构体
struct LB_TMRCB{
	INT16U		interval;	 //时间间隔
	INT16U		cnt;		 //计数器
	INT8U		enable;		 //使能控制位
	INT8U		ok;			 //时间到标记
	TIMER_CB	*fCBack;	 //回调函数
	TIMER_CB	*fCBackInt;
        void            *msg;
	LB_TMRCB	*pNext;		 //链表指针
};

/*内核数据内存，勿修改*/
#ifdef _LB_TIMER_C_
static XRAM LB_TMRCB LB_TMRCBTbl[LB_MAX_TIMER];	//定时器表
#endif

LB_TIMER_EXT void InitLB_TMR(void);	 //初始化定时器
LB_TIMER_EXT LB_TMRCB *LB_TMRCreate(INT16U interval,INT8U enable,TIMER_CB *TMRCallBack,TIMER_CB *TMRCallBackInt,void *msg);	//创建定时器
LB_TIMER_EXT void LB_TMRTick(void);	 //定时器滴嗒
LB_TIMER_EXT void LB_ExcTMR(void);	 //执行定时器
LB_TIMER_EXT void SetLB_TMR(INT16U interval,LB_TMRCB *pTMRCB);	 //设置定时器间隔
LB_TIMER_EXT void EnLB_TMR(LB_TMRCB *pTMRCB);					 //使能定时器
LB_TIMER_EXT void DisLB_TMR(LB_TMRCB *pTMRCB);					 //禁止定时器
LB_TIMER_EXT void ReCntLB_TMR(LB_TMRCB *pTMRCB); 				 //重新计数
LB_TIMER_EXT INT8U GetTimerNum(void);

#endif
