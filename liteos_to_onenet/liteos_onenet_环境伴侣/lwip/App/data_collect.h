#ifndef _DATA_COLLECT_H_
#define _DATA_COLLECT_H_
#include "stm32f4xx.h"

#define MAX_BUF_SIZE 2048


extern u8 CollectDataBuf[MAX_BUF_SIZE];
extern u8 CollectDataLen;
extern u8 CollectDataSendFlag;
u8 getTempHttpData(void);
u8 getLightData(void);

#endif

