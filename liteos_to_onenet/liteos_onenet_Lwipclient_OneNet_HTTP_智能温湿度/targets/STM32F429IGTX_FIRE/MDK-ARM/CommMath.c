#define _COMMMATH_C_
#include "CommMath.h"

/*模块结构体初始化*/
void CommMath_struct_init(void)
{

}

/*模块初始化*/
void CommMath_init(void)
{

   CommMath_struct_init();


}

void MemCopy(BYTE *des,BYTE *src,int len)
{
	int i;
	for(i = 0;i < len;i ++)
	{
		des[i] = src[i];
	}
}

WORD CombineWord(BYTE *buf,int *index)
{
	WORD wData = 0;
	wData |= (((WORD)buf[1]<<8) & 0xff00);
	wData |= (((WORD)buf[0]<<0) & 0x00ff);
	*index += 2;
	return wData;
}

DWORD CombineDword(BYTE *buf,int *index)
{
	DWORD dwData = 0;
	dwData |= (((DWORD)buf[3]<<24) & 0xff000000);
	dwData |= (((DWORD)buf[2]<<16) & 0x00ff0000);
	dwData |= (((DWORD)buf[1]<<8)  & 0x0000ff00);
	dwData |= (((DWORD)buf[0]<<0)  & 0x000000ff);
	*index += 4;
	return dwData;
}

void LoadWord(WORD wData,BYTE *buf,int *index)
{
	buf[0] = (BYTE)(wData & 0xff);
	buf[1] = (BYTE)(wData>>8) & 0xFF;
	*index += 2;
}

void LoadDword(DWORD dwData,BYTE *buf,int *index)
{
	buf[0] = (BYTE)(dwData>>0) & 0xff;
	buf[1] = (BYTE)(dwData>>8) & 0xff;
	buf[2] = (BYTE)(dwData>>16) & 0xff;
	buf[3] = (BYTE)(dwData>>24) & 0xff;
	*index += 4;
}
