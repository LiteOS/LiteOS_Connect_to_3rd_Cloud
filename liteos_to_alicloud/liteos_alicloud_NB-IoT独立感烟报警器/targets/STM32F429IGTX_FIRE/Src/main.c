/* Includes LiteOS------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "los_base.h"
#include "los_config.h"
#include "los_typedef.h"
#include "los_hwi.h"
#include "los_task.ph"
#include "los_sem.h"
#include "los_event.h"
#include "los_memory.h"
#include "los_queue.ph"
#include "cmsis_os.h"
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "bsp_led.h" 
#include "bsp_debug_usart.h"
#include "dwt.h"
#include "bsp_key.h"

#include "bsp_model_usart.h"
#include "bsp_model_nbiot.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */


/* Private function prototypes -----------------------------------------------*/
static void TIM3_Config(uint16_t period,uint16_t prescaler);
/* Private functions ---------------------------------------------------------*/
void TIM3_IRQHandler(void);
void coap_client_example(void);

void hardware_init(void)
{
    LED_GPIO_Config();
    Key1_GPIO_Config();
    Key2_GPIO_Config();
    KeyCreate(&Key1,GetPinStateOfKey1);
    KeyCreate(&Key2,GetPinStateOfKey2);
    Debug_USART_Config();
    DelayInit(SystemCoreClock);
    LOS_HwiCreate(TIM3_IRQn, 0,0,TIM3_IRQHandler,NULL);
    TIM3_Config(999,899);
    printf("Sysclock is %d\r\n",SystemCoreClock);
}


static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  
    
    TIM_TimeBaseInitStructure.TIM_Prescaler=prescaler;  
    TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 
    TIM_TimeBaseInitStructure.TIM_Period=period;   
    TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
    
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
    
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); 
    TIM_Cmd(TIM3,ENABLE); 
    
    NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) 
    {
        LocalTime+=10;
    }
    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  
}


VOID task1()
{
    int count = 0;
    while(1)
    {
        count++;
        printf("------This is task1,count is %d\r\n",count);
       // printf("Mem Max Free Blk Size : %d\r\n",LOS_MemGetMaxFreeBlkSize(OS_SYS_MEM_ADDR));
        LOS_TaskDelay(2000);
    }
}


UINT32 creat_task1()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task1";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task1;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}

VOID task2()
{

    if (model_init() != MODEL_TRUE) {
         return;
    }
    if (model_iot_init() != MODEL_TRUE) {
         return;
    }

    coap_client_example();
    while(1)
    {
        LOS_TaskDelay(1000);
    }
}


UINT32 creat_task2()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task2";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task2;
    task_init_param.uwStackSize = 0x2000;

    uwRet = LOS_TaskCreate(&g_TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}

int main(void)
{
    UINT32 uwRet = LOS_OK;
    LOS_KernelInit();//内核初始化   
    hardware_init();//硬件初始化
    uwRet = creat_task1();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
        
    uwRet = creat_task2();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }

    LOS_Start();//启动LiteOS
}
