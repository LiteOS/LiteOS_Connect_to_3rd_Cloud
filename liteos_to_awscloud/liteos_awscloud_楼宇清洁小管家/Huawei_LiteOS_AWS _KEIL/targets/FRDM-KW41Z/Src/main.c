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

#include "los_inspect_entry.h"
#include "los_demo_entry.h"
#include "los_bsp_adapter.h"

#include "board.h"
#include "fsl_lpuart.h"

#include "pin_mux.h"
#include "clock_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


#define DEMO_LPUART LPUART0
#define DEMO_LPUART_CLKSRC kCLOCK_CoreSysClk
#define DEMO_LPUART_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)


#ifndef LOS_KERNEL_TEST_KEIL_SWSIMU
struct __FILE 
{
    int handle; /* Add whatever needed */ 
}; 

FILE __stdout;
FILE __stdin; 

int fputc(int ch, FILE *f) 
{ 
    LPUART_WriteBlocking(DEMO_LPUART, (uint8_t *)&ch, 1);
    return ch;
}

#endif  

void LOS_UartInit(void)
{
    lpuart_config_t config;

    BOARD_InitPins();
    CLOCK_SetLpuartClock(0x1U);

    /*
     * config.baudRate_Bps = 1000000U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 1000000;
    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(DEMO_LPUART, &config, DEMO_LPUART_CLK_FREQ);
}

int main(void)
{
    UINT32 uwRet = LOS_OK;
    
    BOARD_BootClockRUN();
    
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    
    LOS_EvbSetup();
    
    LOS_UartInit();
    
    LOS_Inspect_Entry();
		
    LOS_Start();
}
