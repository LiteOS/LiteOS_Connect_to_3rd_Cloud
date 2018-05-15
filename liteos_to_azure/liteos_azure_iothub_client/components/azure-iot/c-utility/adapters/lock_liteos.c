// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"


LOCK_HANDLE Lock_Init(void)
{
    /* Codes_SRS_LOCK_10_002: [Lock_Init on success shall return a valid lock handle which should be a non NULL value] */
    /* Codes_SRS_LOCK_10_003: [Lock_Init on error shall return NULL ] */
    // xSemaphoreCreateMutex is the obvious choice, but it returns a recursive
    // sync object, which we do not want for the lock adapter.
    UINT32 usSemID;
    if (LOS_BinarySemCreate(0, &usSemID) == LOS_OK)
    {
        if (LOS_SemPost(usSemID) != LOS_OK)
        {
            LogError("Lock_Init LOS_SemPost failed after create.");
            (void)LOS_SemDelete(usSemID);
            usSemID = LOSCFG_BASE_IPC_SEM_LIMIT;
        }
    }
    else
    {
        LogError("Lock_Init LOS_BinarySemCreate failed.");
        usSemID = LOSCFG_BASE_IPC_SEM_LIMIT;
    }
    return usSemID;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    LOCK_RESULT result;

    if (handle >= LOSCFG_BASE_IPC_SEM_LIMIT)
    {
        LogError("Lock_Deinit Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        (void)LOS_SemDelete(handle);
        result = LOCK_OK;
    }

    return result;
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;

    if (handle >= LOSCFG_BASE_IPC_SEM_LIMIT)
    {
        LogError("Lock Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        if (LOS_SemPend(handle, LOS_WAIT_FOREVER) == LOS_OK)
        {
            result = LOCK_OK;
        }
        else
        {
            LogError("Lock LOS_SemPend failed.");
            /* Codes_SRS_LOCK_10_006: [Lock on error shall return LOCK_ERROR] */
            result = LOCK_ERROR;
        }
    }

    return result;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;

    if (handle >= LOSCFG_BASE_IPC_SEM_LIMIT)
    {
        LogError("Unlock Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        if (LOS_SemPost(handle) == LOS_OK)
        {
            result = LOCK_OK;
        }
        else
        {
            LogError("Unlock LOS_SemPost failed.");
            /* Codes_SRS_LOCK_10_006: [Lock on error shall return LOCK_ERROR] */
            result = LOCK_ERROR;
        }
    }
    
    return result;
}
