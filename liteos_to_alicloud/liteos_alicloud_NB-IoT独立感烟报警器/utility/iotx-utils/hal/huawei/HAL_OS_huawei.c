/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <sys/time.h>
#include "los_memory.h"

//#include <aos/kernel.h>
#include "iot_import.h"
//#include "aliot_platform_os.h"

#define PLATFORM_LINUX_LOG(format, ...) \
    do { \
        printf("Linux:%s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }while(0);

void *HAL_MutexCreate(void)
{
    UINT32 * mutex;
    if (0 != LOS_MuxCreate(&mutex)) {
        return NULL;
    }

    return mutex;
    /*
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex)
    {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        perror("create mutex failed");
        free(mutex);
        return NULL;
    }

    return mutex;*/
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    LOS_MuxDelete((UINT32 *)&mutex);
    /*
    int err_num;
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        perror("destroy mutex failed");
    }

    free(mutex);*/
}

void HAL_MutexLock(_IN_ void *mutex)
{
    LOS_MuxPend((UINT32 *)&mutex, 10000);
    /*
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        perror("lock mutex failed");
    }*/

}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    LOS_MuxPost((UINT32 *)&mutex);
    /*
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        perror("unlock mutex failed");
    }*/
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return LOS_MemAlloc(OS_SYS_MEM_ADDR, size);
    //return malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    LOS_MemFree(OS_SYS_MEM_ADDR, ptr);
    //return free(ptr);
}

uint32_t HAL_UptimeMs(void)
{
    return 0;
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    LOS_TaskDelay(ms);
    //usleep( 1000 * ms );
}

void HAL_Printf(_IN_ const char *fmt, ...)
{

}

char *HAL_GetPartnerID(char pid_str[])
{
    return NULL;
}
