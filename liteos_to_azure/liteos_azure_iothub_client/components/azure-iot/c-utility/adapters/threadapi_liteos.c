// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdlib.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"
#include "los_task.h"

DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
	LogError("LiteOS does not support multi-thread function.");
    return THREADAPI_ERROR;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res)
{
    LogError("LiteOS does not support multi-thread function.");
    return THREADAPI_ERROR;
}

void ThreadAPI_Exit(int res)
{
    (void)LOS_TaskDelete(LOS_CurTaskIDGet());
}

void ThreadAPI_Sleep(unsigned int milliseconds)
{
    (void)LOS_TaskDelay(LOS_MS2Tick(milliseconds));
}
