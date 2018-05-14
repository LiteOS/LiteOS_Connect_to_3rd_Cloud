/**
  ******************************************************************************
  * @file    bsp_rng.c
  * @author  游博
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   rng应用函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_rng.h"   
#include "dwt.h"
 /**
  * @brief  初始化硬件随机数rng
  * @param  无
  * @retval 无
  */
int RNG_Config(void)
{		
	uint32_t retry = 0;
    /* 使能RNG时钟 */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
        
    /* 使能RNG外设 */
	RNG_Cmd(ENABLE);
        

	/* 等待随机数产生完毕 */
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET && (retry < 10000))
	{
		retry++;
		Delayus(100);
	}
	if(retry >= 10000)
	{
		return -1;
	}
	return 0;
}

uint32_t RNG_Get_RandomNum(void)
{
	uint32_t random;
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET);
	/*获取随机数*/       
	random = RNG_GetRandomNumber();                       
	printf("\r\nRNG:%08x",random);
	return random;
		
}


int RNG_Get_RandomRange(int min,int max)
{       
	return RNG_Get_RandomNum()%(max - min + 1) + min;                       		
}
/*********************************************END OF FILE**********************/
