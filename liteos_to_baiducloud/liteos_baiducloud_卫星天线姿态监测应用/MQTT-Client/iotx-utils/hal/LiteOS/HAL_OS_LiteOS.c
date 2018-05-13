/*
 * Copyright (C) 2018-2019 Áõºé·å@Ò¶·«¿Æ¼¼   Î¢ÐÅ£ºyefanqiu
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "iot_import.h"
#include "los_base.h"
#include "los_mux.h"
#include "los_config.h"
#include "los_memory.h"

int errno;
void __aeabi_assert(const char *expr, const char *file, int line)
{
    while (1);
}

void *HAL_MutexCreate(void)
{	
	  UINT32 puwMuxHandle;
	  UINT32 ret = LOS_MuxCreate(&puwMuxHandle);
   
    if (LOS_OK != ret) {
        return NULL;
    }
    return (void *)(puwMuxHandle+1);    
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
	  UINT32 puwMuxHandle = (UINT32)mutex;
    LOS_MuxDelete(puwMuxHandle-1);
}

void HAL_MutexLock(_IN_ void *mutex)
{
	  UINT32 puwMuxHandle = (UINT32)mutex;
	  LOS_MuxPend(puwMuxHandle-1,LOS_WAIT_FOREVER);
}

void HAL_MutexUnlock(_IN_ void *mutex)
{	
	  UINT32 puwMuxHandle = (UINT32)mutex;
	  LOS_MuxPost(puwMuxHandle-1);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return (void *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR,size);
}

void HAL_Free(_IN_ void *ptr)
{
    LOS_MemFree((VOID *)OS_SYS_MEM_ADDR,ptr);
}

uint64_t HAL_UptimeMs(void)
{
	  UINT64 tick = LOS_TickCountGet();	  
    return LOS_Tick2MS((UINT32)tick);
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
   LOS_TaskDelay(ms);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

int HAL_GetPartnerID(char pid_str[])
{
    return NULL;
}

int HAL_GetModuleID(char mid_str[64])
{
    memset(mid_str, 0x0, 64);
    return strlen(mid_str);
}

void HAL_Srandom(uint32_t seed)
{
}

uint32_t HAL_Random(uint32_t region)
{
    return  0; //(region > 0) ? (random() % region) : 0;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;
    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

