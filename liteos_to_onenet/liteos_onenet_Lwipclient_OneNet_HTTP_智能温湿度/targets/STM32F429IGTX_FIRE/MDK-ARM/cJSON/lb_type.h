#ifndef _LB_TYPE_H_
#define _LB_TYPE_H_

typedef unsigned char  	INT8U;         //无符号8位数
typedef signed   char  	INT8S;         //有符号8位数
typedef unsigned short  INT16U;        //无符号16位数
typedef signed   short  INT16S;        //有符号16位数
typedef unsigned int    INT32U;        //有符号16位数

typedef INT8U			uchar;
typedef INT16U			uint;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;

#ifndef TRUE
#define TRUE  (1==1)
#endif

#ifndef FALSE
#define FALSE (1==0)
#endif

#ifndef NULL
	#define NULL (void *)0	   /*空指针*/
#endif

#define __In_
#define __Out_
#define __Inout_

#endif
