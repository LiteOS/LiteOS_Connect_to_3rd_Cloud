/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "include.h"

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
    LOS_MuxDelete((UINT32)mutex-1);
}

void HAL_MutexLock(_IN_ void *mutex)
{
	  LOS_MuxPend((UINT32)mutex-1,LOS_WAIT_FOREVER);
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
	  LOS_MuxPost(((UINT32)mutex)-1);
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
    return LOS_Tick2MS(LOS_TickCountGet());
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
    return 0;
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

int errno;

