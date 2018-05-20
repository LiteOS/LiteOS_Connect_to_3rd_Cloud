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
*History	:2015/7/1 22:22:27
***********************************************************************************
*/
#ifndef _COMMMATH_H_
#define _COMMMATH_H_

#include "lb_type.h"

#ifdef _COMMMATH_C_
#define COMMMATH_EXT
#else 
#define COMMMATH_EXT extern
#endif 

#ifdef _COMMMATH_C_
#endif 

COMMMATH_EXT void MemCopy(BYTE *des,BYTE *src,int len);
COMMMATH_EXT WORD CombineWord(BYTE *buf,int *index);
COMMMATH_EXT DWORD CombineDword(BYTE *buf,int *index);
COMMMATH_EXT void LoadWord(WORD wData,BYTE *buf,int *index);
COMMMATH_EXT void LoadDword(DWORD dwData,BYTE *buf,int *index);	
#endif
