#include "los_sys.h"
#include "los_tick.h"
#include "los_task.ph"
#include "los_config.h"

#include "los_bsp_led.h"
#include "los_bsp_key.h"
#include "los_bsp_uart.h"
#include "los_inspect_entry.h"
#include "los_demo_entry.h"
#include "usr_cfg.h"
#include <string.h>

extern void LOS_EvbSetup(void);


/*****************************************************************************
Function    : main
Description : Main function entry
Input       : None
Output      : None
Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT
int main(void)
{
    UINT32 uwRet;

    /*Init LiteOS kernel */
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK) {
        return LOS_NOK;
    }
    
    /* Enable LiteOS system tick interrupt */
    LOS_EnableTick();

    /*
        Notice: add your code here
        here you can create task for your function 
        do some hw init that need after systemtick init
     */
    LOS_EvbSetup();//init the device on the dev baord
    printf("this is LiteOS lwip port \r\n");

    
    /* Kernel start to run */
    LOS_Start();
    for (;;);
    /* Replace the dots (...) with your own code. */
}
