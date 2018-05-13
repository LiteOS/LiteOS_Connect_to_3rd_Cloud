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
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

int main(void)
{
    UINT32 uwRet = LOS_OK;
    
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK)
    {
        return LOS_NOK;
    }
    
    LOS_EvbSetup();
    
    LOS_Inspect_Entry();
		
    LOS_Start();
}
