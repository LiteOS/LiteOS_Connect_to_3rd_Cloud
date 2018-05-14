#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "los_bsp_led.h"

#define LED1_PIN                  GPIO_Pin_10                 
#define LED1_GPIO_PORT            GPIOH                      
#define LED1_GPIO_CLK             RCC_AHB1Periph_GPIOH

#define LED2_PIN                  GPIO_Pin_11                 
#define LED2_GPIO_PORT            GPIOH                      
#define LED2_GPIO_CLK             RCC_AHB1Periph_GPIOH

#define LED3_PIN                  GPIO_Pin_12                  
#define LED3_GPIO_PORT            GPIOH                       
#define LED3_GPIO_CLK             RCC_AHB1Periph_GPIOH

/* 带参宏，可以像内联函数一样使用 */
#define LED1(a)	if (a)	\
					GPIO_SetBits(LED1_GPIO_PORT,LED1_PIN);\
					else		\
					GPIO_ResetBits(LED1_GPIO_PORT,LED1_PIN)

#define LED2(a)	if (a)	\
					GPIO_SetBits(LED2_GPIO_PORT,LED2_PIN);\
					else		\
					GPIO_ResetBits(LED2_GPIO_PORT,LED2_PIN)

#define LED3(a)	if (a)	\
					GPIO_SetBits(LED3_GPIO_PORT,LED3_PIN);\
					else		\
					GPIO_ResetBits(LED3_GPIO_PORT,LED3_PIN)

/* 直接操作寄存器的方法控制IO */
#define	digitalHi(p,i)			   {p->BSRRL=i;}			  //设置为高电平		
#define digitalLo(p,i)			   {p->BSRRH=i;}				//输出低电平
#define digitalToggle(p,i)		 {p->ODR ^=i;}			//输出反转状态

/* 定义控制IO的宏 */
#define LED1_TOGGLE    digitalToggle(LED1_GPIO_PORT,LED1_PIN)
#define LED1_OFF       digitalHi(LED1_GPIO_PORT,LED1_PIN)
#define LED1_ON        digitalLo(LED1_GPIO_PORT,LED1_PIN)

#define LED2_TOGGLE    digitalToggle(LED2_GPIO_PORT,LED2_PIN)
#define LED2_OFF       digitalHi(LED2_GPIO_PORT,LED2_PIN)
#define LED2_ON        digitalLo(LED2_GPIO_PORT,LED2_PIN)

#define LED3_TOGGLE    digitalToggle(LED3_GPIO_PORT,LED3_PIN)
#define LED3_OFF       digitalHi(LED3_GPIO_PORT,LED3_PIN)
#define LED3_ON        digitalLo(LED3_GPIO_PORT,LED3_PIN)

//LED1 - R
//LED2 - G
//LED3 - B
                                       //  R   G   B
#define  LED_BLACK             0X0     //  0   0   0
#define  LED_RED               0X1     //  0   0   1
#define  LED_GREEN             0X2     //  0   1   0
#define  LED_YELLOW            0X3     //  0   1   1
#define  LED_BLUE              0X4     //  1   0   0
#define  LED_MAGENTA           0X5     //  1   0   1
#define  LED_CYAN              0X6     //  1   1   0
#define  LED_WHITE             0X7     //  1   1   1

#define IS_RGB111_ALL_PERIPH(PERIPH) (((PERIPH) == LED_BLACK)   || \
																			((PERIPH) == LED_RED)     || \
																			((PERIPH) == LED_GREEN)   || \
																			((PERIPH) == LED_YELLOW)  || \
																			((PERIPH) == LED_BLUE)    || \
																			((PERIPH) == LED_MAGENTA) || \
																			((PERIPH) == LED_CYAN)    || \
																			((PERIPH) == LED_WHITE)

void LOS_EvbLedInit(void)
{
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /*开启LED相关的GPIO外设时钟*/
    RCC_AHB1PeriphClockCmd ( LED1_GPIO_CLK|LED2_GPIO_CLK|LED3_GPIO_CLK, ENABLE); 
    
    /*选择要控制的GPIO引脚*/
    GPIO_InitStructure.GPIO_Pin = LED1_PIN;
    
    /*设置引脚模式为输出模式*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    
    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    
    /*设置引脚为上拉模式，默认LED亮*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    
    /*设置引脚速率为50MHz */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    /*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
    GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);
    
    /*选择要控制的GPIO引脚*/
    GPIO_InitStructure.GPIO_Pin = LED2_PIN;
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);
    
    /*选择要控制的GPIO引脚*/
    GPIO_InitStructure.GPIO_Pin = LED3_PIN;
    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);
    
    /* 关闭所有LED */
    GPIO_SetBits(LED1_GPIO_PORT,LED1_PIN);
    GPIO_SetBits(LED2_GPIO_PORT,LED2_PIN);
    GPIO_SetBits(LED3_GPIO_PORT,LED3_PIN);

    return;
}

/*************************************************************************************************
 *  function：control led on off                                                                 *
 *  param (1) index Led's index                                                                  *
 *        (2) cmd   Led on or off                                                                *
 *  return : None                                                                                *
 *  discription:                                                                                 *
**************************************************************************************************/
void LOS_EvbLedControl(int index, int cmd)
{
    switch (index)
    {
        case LOS_LED1:
        {
            if (cmd == LED_ON)
            {
                LED1(1); /*led1 on */
            }
            else
            {
                LED1(0); /*led1 off */
            }
            break;
        }
        case LOS_LED2:
        {
            if (cmd == LED_ON)
            {
                LED2(1); /*led2 on */
            }
            else
            {
                LED2(0); /*led2 off */
            }
            break;
        }
        default:
        {
            break;
        }
    }
    
    return;
}


