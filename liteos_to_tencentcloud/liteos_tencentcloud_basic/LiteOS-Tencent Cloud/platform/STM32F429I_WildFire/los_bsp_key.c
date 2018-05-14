#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "los_bsp_key.h"

//定义两个按键的引脚
#define KEY1_PIN                  GPIO_Pin_0                 
#define KEY1_GPIO_PORT            GPIOA                      
#define KEY1_GPIO_CLK             RCC_AHB1Periph_GPIOA

void LOS_EvbKeyInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;	
	/*开启按键GPIO口的时钟*/
	RCC_AHB1PeriphClockCmd(KEY1_GPIO_CLK,ENABLE);
    /*选择按键的引脚*/
	GPIO_InitStructure.GPIO_Pin = KEY1_PIN; 
    /*设置引脚为输入模式*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
    /*设置引脚不上拉也不下拉*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	/* 设置引脚速度 */
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_2MHz;
    /*使用上面的结构体初始化按键*/
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);

    return;
}

/*****************************************************************************
 Function    : LOS_EvbGetKeyVal
 Description : Get GIOP Key value
 Input       : int KeyNum
 Output      : None
 Return      : KeyVal
 *****************************************************************************/
unsigned int LOS_EvbGetKeyVal(int KeyNum)
{
    unsigned int KeyVal = LOS_GPIO_ERR; 

    if(GPIO_ReadInputDataBit(KEY1_GPIO_PORT,KEY1_PIN)==1)
        KeyVal = LOS_KEY_PRESS;
    //add you code here.

    return KeyVal;
}

