#ifndef __RNG_H
#define	__RNG_H

#include "stm32f4xx.h"


int RNG_Config(void);
uint32_t RNG_Get_RandomNum(void);
int RNG_Get_RandomRange(int min,int max);


#endif /* __RNG_H */