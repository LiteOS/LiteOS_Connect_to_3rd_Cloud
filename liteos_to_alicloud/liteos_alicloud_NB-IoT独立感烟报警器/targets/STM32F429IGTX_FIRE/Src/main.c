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
#include "bsp_adc.h"
#include "bsp_model_usart.h"
#include "bsp_model_nbiot.h"
#include "iot_import.h"
#include "iot_export.h"
#include "CoAPExport.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define IOTX_ONLINE_DTLS_SERVER_URL     "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#define IOTX_PRODUCT_KEY         "a11h2ArCAS1" //"vtkkbrpmxmF"
#define IOTX_DEVICE_NAME         "SDM200_DD" //"IoTxCoAPTestDev"
#define IOTX_DEVICE_SECRET       "ndovdWS8ZLYwOtsKp2BPnAofxlyejxsF" //"Stk4IUErQUBc1tWRWEKWb5ACra4hFDYF"
#define IOTX_DEVICE_ID           "SDM200_DD" //"IoTxCoAPTestDev.1"

#define BUF_IO_SIZE 2
#define BUF_AD_SIZE 2

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
UINT32 g_TskHandle[3];
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */

UINT32 g_uwQueue_io;
char buf_io[BUF_IO_SIZE] = {0};

/* Private function prototypes -----------------------------------------------*/
static void TIM3_Config(uint16_t period,uint16_t prescaler);
/* Private functions ---------------------------------------------------------*/
void TIM3_IRQHandler(void);
void hardware_init(void)
{
    LED_GPIO_Config();
    Key1_GPIO_Config();
    Key2_GPIO_Config();
    KeyCreate(&Key1,GetPinStateOfKey1);
    KeyCreate(&Key2,GetPinStateOfKey2);
    Rheostat_ADC_Init();
    Debug_USART_Config();
    DelayInit(SystemCoreClock);
    LOS_HwiCreate(TIM3_IRQn, 0,0,TIM3_IRQHandler,NULL);
    TIM3_Config(999,899);
    LOS_HwiCreate(MODEL_USART_IRQ, 0,0,MODEL_USART_IRQHandler,NULL);
    //MODEL_USART_Config();
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


void HWSmokeCheck()
{
    static UINT32 count = 0;
    UINT32 uwRet = LOS_OK;

    Key_RefreshState(&Key2);

    if(CHANGED == Key_AccessState(&Key2,&(Key2.StateChange)))
    {
        count++;

        if(Key_AccessTimes(&Key2,KEY_ACCESS_READ) > 0)
        {
            if((count%2) == 0)
            {
                printf("Smoke Alarm\r\n");
                buf_io[0] = 1;
            }
            else
            {
                printf("Smoke Normal\r\n");
                buf_io[0] = 0;
            }

            uwRet = LOS_QueueWrite(g_uwQueue_io, buf_io, BUF_IO_SIZE, 0);
        }
        
        if (uwRet != LOS_OK)
        {
            printf("LOS_QueueWrite failure uwRet = %d\r\n",uwRet);
        }
    }
}

void HWRemoveCheck()
{
    static UINT32 count = 0;
    static UINT64 preTime = 0;
    UINT32 uwRet = LOS_OK;
    static UINT8 writeFlag = FALSE;

    Key_RefreshState(&Key1);

    if(CHANGED == Key_AccessState(&Key1,&(Key1.StateChange)))
    {
        count++;

        if(Key_AccessTimes(&Key1,KEY_ACCESS_READ) > 0)
        {
            if((count%2) == 0)
            {
                buf_io[1] = 0;
            }
            else
            {
                buf_io[1] = 1;
            }

            preTime = LOS_TickCountGet();
            writeFlag = TRUE;
        }
    }

    if((writeFlag == TRUE) && (LOS_TickCountGet() - preTime > 100))
    {
        if (buf_io[1] == 0) {
            printf("Remove Normal\r\n");
        } else {
            printf("Rmove Alarm\r\n");
        }
    
        uwRet = LOS_QueueWrite(g_uwQueue_io, buf_io, BUF_IO_SIZE, 0);
        if(uwRet != LOS_OK)
        {
            printf("LOS_QueueWrite failure uwRet = %d\r\n",uwRet);
        }

        writeFlag = FALSE;
    }
}

UINT8 HWGetBatVolPercent()
{
    uint16_t ADC_ConvertedValue_0;
    UINT8 vol_percent;
    
    ADC_ConvertedValue_0 = Rheostat_ADC_StartConv();
    printf("ADC_ConvertedValue_0 0x%04X \r\n", ADC_ConvertedValue_0);
    vol_percent =(UINT8)(ADC_ConvertedValue_0*100/4096);
    printf("vol_percent %d \r\n", vol_percent);
    Rheostat_ADC_EndConv();

    return vol_percent;
}

VOID task_hw_io()
{
    while(1)
    {
        HWSmokeCheck();
        HWRemoveCheck();
    }
    
}

UINT32 creat_task_hw_io()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task_io";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task_hw_io;
    task_init_param.uwStackSize = 0x400;

    uwRet = LOS_TaskCreate(&g_TskHandle[0], &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}

extern void bc95_nbiot_module_socket_data_handle(void);
VOID task_hw_bc95()
{
    while(1)
    {
        LOS_TaskDelay(50);
        bc95_nbiot_module_socket_data_handle();
    }
}


UINT32 creat_task_hw_bc95()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task_bc95";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task_hw_bc95;
    task_init_param.uwStackSize = 0x1000;

    uwRet = LOS_TaskCreate(&g_TskHandle[1], &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}

static int set_devinfo(iotx_deviceinfo_t *p_devinfo)
{
    if (NULL == p_devinfo) {
        return IOTX_ERR_INVALID_PARAM;
    }

    memset(p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    strncpy(p_devinfo->device_id,    IOTX_DEVICE_ID,   IOTX_DEVICE_ID_LEN);
    strncpy(p_devinfo->product_key,  IOTX_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
    strncpy(p_devinfo->device_secret, IOTX_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
    strncpy(p_devinfo->device_name,  IOTX_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);

    fprintf(stderr, "*****The Product Key  : %s *****\r\n", p_devinfo->product_key);
    fprintf(stderr, "*****The Device Name  : %s *****\r\n", p_devinfo->device_name);
    fprintf(stderr, "*****The Device Secret: %s *****\r\n", p_devinfo->device_secret);
    fprintf(stderr, "*****The Device ID    : %s *****\r\n", p_devinfo->device_id);
    return IOTX_SUCCESS;
}

static iotx_coap_context_t * coap_client_init(void)
{
    iotx_coap_context_t *p_ctx;
    iotx_coap_config_t config;
    iotx_deviceinfo_t deviceinfo;
    
    set_devinfo(&deviceinfo);
#ifdef COAP_DTLS_SUPPORT
    char url[256] = {0};
    snprintf(url, sizeof(url), IOTX_ONLINE_DTLS_SERVER_URL, deviceinfo.product_key);
    config.p_url = url;
#else
    LOG("Online environment must access with DTLS\r\n");
    return -1;
#endif

    config.p_devinfo = &deviceinfo;

    p_ctx = IOT_CoAP_Init(&config);

    return p_ctx;
}

extern int sal_device_init(void);
extern int sal_init(void);
VOID task_aliyun()
{
    iotx_coap_context_t *p_context;
    static int auth_flag;
    int ret;
    static UINT64 last_tick;
    int period = 30;
    static char smoke = 0, remove = 1, bat;
    UINT32 uwRet = LOS_OK;
    UINT32 uwReadbuf;
    UINT32 addr;
    char alarm = 0;

    sal_device_init();
    if (sal_init() >=0 ) {
        p_context = coap_client_init();
        
        if (NULL != p_context) {
            auth_flag = IOT_CoAP_DeviceNameAuth(p_context);
            last_tick = LOS_TickCountGet();
        }
    }
    
    while (1) {
        if (auth_flag == IOTX_SUCCESS) {
        
            uwRet = LOS_QueueRead(g_uwQueue_io, &uwReadbuf, 24, 0);
            if (uwRet == LOS_OK) {
                addr = uwReadbuf;
                smoke = *((UINT8 *)(addr+0));
                remove = *((UINT8 *)(addr+1));
                if ((smoke > 0) || (remove > 0)) {
                    alarm = 1;
                }
            }

            if (((LOS_TickCountGet() - last_tick) > (period * 1000)) || (alarm > 0))
            {
                bat = HWGetBatVolPercent();
                COAP_DEBUG("report_prop %d %d %d\r\n", smoke, remove, bat);
                ret = iotx_coap_report_prop(p_context, smoke, remove, bat, period);
                if (SUCCESS_RETURN != ret) {
                    COAP_DEBUG("Send ModuleId message to server(CoAP) failed ret = %d", ret);
                }
                last_tick = LOS_TickCountGet();
                alarm = 0;
            }
        }
    }
}


UINT32 creat_task_aliyun()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 0;
    task_init_param.pcName = "task_aliyun";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)task_aliyun;
    task_init_param.uwStackSize = 0xA000;

    uwRet = LOS_TaskCreate(&g_TskHandle[2], &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
        
}

int main(void)
{
    UINT32 uwRet = LOS_OK;
    LOS_KernelInit();  
    hardware_init();

    uwRet = creat_task_hw_io();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }

    uwRet = creat_task_hw_bc95();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
        
    uwRet = creat_task_aliyun();
    if(uwRet != LOS_OK)
    {
        return uwRet;
    }
  
    LOS_Start();//Æô¶¯LiteOS
}
