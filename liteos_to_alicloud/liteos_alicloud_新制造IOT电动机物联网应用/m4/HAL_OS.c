/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
//#include <stdarg.h>

#include "iot_import.h"
#include "los_config.h"

/** used for OS time - stNowTime */
#include "../modbus.h"

extern TDateTime stNowTime;

#define MUX_NULL_TIAN
#ifdef MUX_NULL_TIAN
//#undef MUX_NULL_TIAN
#endif 

MUX_CB_S *HAL_MutexCreate(void)
{
#ifdef MUX_NULL_TIAN
    u32_t uMuxId;
    u32_t uRet;
    
    uRet = LOS_MuxCreate(&uMuxId);

    if(LOS_OK == uRet)
    {
        return (MUX_CB_S *)GET_MUX(uMuxId);
    }
    else
    {
        return NULL;
    }
#endif 
    return NULL;
}

void HAL_MutexDestroy(_IN_ MUX_CB_S *mutex)
{    
#ifdef MUX_NULL_TIAN
    u32_t uRet;

    if (NULL == mutex)
    {
        return;
    }

    uRet = LOS_MuxDelete(((MUX_CB_S*)mutex)->ucMuxID);
#endif     
}

void HAL_MutexLock(_IN_ MUX_CB_S *mutex)
{
#ifdef MUX_NULL_TIAN
    u32_t uRet;

    if (NULL == mutex)
    {
        return;
    }

    /** LOS_WAIT_FOREVER方式申请互斥锁,获取不到时程序阻塞，不会返回 */
    uRet = LOS_MuxPend(((MUX_CB_S*)mutex)->ucMuxID, LOS_WAIT_FOREVER);
#endif 
}

void HAL_MutexUnlock(_IN_ MUX_CB_S *mutex)
{
#ifdef MUX_NULL_TIAN
    u32_t uRet;

    if (NULL == mutex)
    {
        return;
    }

    uRet = LOS_MuxPost(((MUX_CB_S*)mutex)->ucMuxID);
#endif     
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    mem_malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    mem_free(ptr);
}

uint32_t HAL_UptimeMs(void)
{
    uint32_t time_ms;

    /*(stNowTime.y-18) 暂不考虑 年 月 日 的转换 */
    time_ms = (stNowTime.h*3600 + stNowTime.min*60 + stNowTime.s) *1000 + stNowTime.ms;

    return time_ms;
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
//    if ((ms > 0) && (ms < (LOSCFG_BASE_CORE_TICK_PER_SECOND/1000))) {
//        ms = (LOSCFG_BASE_CORE_TICK_PER_SECOND/1000);
//    }
//    LOS_TaskDelay(ms / (LOSCFG_BASE_CORE_TICK_PER_SECOND/1000));
    /** because of 1 tick = 1ms */
    LOS_TaskDelay(ms);
}

char *HAL_GetPartnerID(char pid_str[])
{
    return NULL;
}
