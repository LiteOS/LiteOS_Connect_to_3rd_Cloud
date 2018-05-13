/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: xiaxiaowen https://github.com/xiaowenxia
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <lwip/opt.h>
#include <lwip/arch.h>
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>
#include <arch/sys_arch.h>

/* These functions are used for LiteOS only */

/*---------------------------------------------------------------------------*
 * Routine:  sys_init
 *---------------------------------------------------------------------------*
 * Description:
 *      doing lwip sys init,for LiteOS,nothing need to init
 * Inputs:
 * Outputs:
 *---------------------------------------------------------------------------*/
void sys_init(void)
{
    /* do nothing */
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_now
 *---------------------------------------------------------------------------*
 * Description:
 *      return the systick counter
 * Inputs:
 * Outputs:
 *      u32_t                   -- systick count
 *---------------------------------------------------------------------------*/
u32_t sys_now(void)
{
	return (u32_t)LOS_TickCountGet();
}

#if !NO_SYS

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Creates and returns a new semaphore. The "count" argument specifies
 *      the initial state of the semaphore.
 *      NOTE: Currently this routine only creates counts of 1 or 0
 * Inputs:
 *      sys_sem_t *sem          -- Handle of semaphore
 *      u8_t count              -- Initial count of semaphore (1 or 0)
 * Outputs:
 *      err_t                   -- ERR_MEM: create semaphore failed
                                   ERR_OK:  create semaphore success
 *---------------------------------------------------------------------------*/
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    err_t ret = ERR_MEM;

    /* create mutex */
    if(LOS_MuxCreate(sem) != LOS_OK)
    {
        ret = ERR_MEM;
        //SYS_STATS_INC(sem.err);
    }
    else
    {
        ret = ERR_OK;
        //SYS_STATS_INC_USED(sem);
    }

    if(count == 0UL)
    {
        /* get the mutex */
        if(LOS_MuxPend(*sem, LOS_WAIT_FOREVER) != LOS_OK)
        {
            ret = ERR_MEM;
            //SYS_STATS_INC(sem.err);
        }
        else
        {
            ret = ERR_OK;
        }
    }

    return ret;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_free
 *---------------------------------------------------------------------------*
 * Description:
 *      Deallocates a semaphore
 * Inputs:
 *      sys_sem_t *sem           -- Semaphore to free
 *---------------------------------------------------------------------------*/
void sys_sem_free(sys_sem_t *sem)
{
    /* parameter check */
    LWIP_ASSERT("sem != NULL", sem != NULL);
    
    // SYS_STATS_DEC(sem.used);
    LOS_MuxDelete(*sem);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_signal
 *---------------------------------------------------------------------------*
 * Description:
 *      Signals (releases) a semaphore
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to signal
 *---------------------------------------------------------------------------*/
void sys_sem_signal(sys_sem_t *sem)
{
    /* parameter check */
    LWIP_ASSERT("sem != NULL", sem != NULL);

    LOS_MuxPost(*sem);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_sem_wait
 *---------------------------------------------------------------------------*
 * Description:
 *      Blocks the thread while waiting for the semaphore to be
 *      signaled. If the "timeout" argument is non-zero, the thread should
 *      only be blocked for the specified time (measured in
 *      milliseconds).
 *
 *      If the timeout argument is non-zero, the return value is the number of
 *      milliseconds spent waiting for the semaphore to be signaled. If the
 *      semaphore wasn't signaled within the specified time, the return value is
 *      SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
 *      (i.e., it was already signaled), the function may return zero.
 *
 *      Notice that lwIP implements a function with a similar name,
 *      sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to wait on
 *      u32_t timeout           -- Number of milliseconds until timeout
 * Outputs:
 *      u32_t                   -- Time elapsed or SYS_ARCH_TIMEOUT.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    u32_t ret;
    UINT64 time_start, time_end, time_diff;

    /* parameter check */
    LWIP_ASSERT("sem != NULL", sem != NULL);

    time_start = LOS_TickCountGet();
    
    if( timeout != 0UL )
    {
        if(LOS_MuxPend(*sem, timeout) == LOS_OK)
        {
            time_end = LOS_TickCountGet();
            time_diff = time_end - time_start;
            ret = time_diff;
        }
        else
        {
            ret = SYS_ARCH_TIMEOUT;
        }
    }
    else
    {
        LOS_MuxPend(*sem, LOS_WAIT_FOREVER);
        time_end = LOS_TickCountGet();
        time_diff = time_end - time_start;
        ret = time_diff;
    }

    return ret;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mutex_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Creates and returns a new mutex.
 *      just call sys_mutex_new()
 * Inputs:
 *      sys_mutex_t *mutex          -- Handle of mutex=
 * Outputs:
 *      err_t                   -- ERR_MEM: create mutex failed
                                   ERR_OK:  create mutex success
 *---------------------------------------------------------------------------*/
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    return sys_sem_new((sys_sem_t *)mutex, 1);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mutex_free
 *---------------------------------------------------------------------------*
 * Description:
 *      Deallocates a mutex
 *      just call sys_sem_free()
 * Inputs:
 *      sys_mutex_t *mutex           -- mutex to free
 *---------------------------------------------------------------------------*/
void sys_mutex_free(sys_mutex_t *mutex)
{
    sys_sem_free((sys_sem_t *)mutex);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mutex_lock
 *---------------------------------------------------------------------------*
 * Description:
 *      wait a mutex
 *      just call sys_arch_sem_wait()
 * Inputs:
 *      sys_mutex_t mutex           -- mutex to lock
 *---------------------------------------------------------------------------*/
void sys_mutex_lock(sys_mutex_t *mutex)
{
    sys_arch_sem_wait((sys_sem_t *)mutex, 0);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mutex_unlock
 *---------------------------------------------------------------------------*
 * Description:
 *      unlock (releases) a mutex
 *      just call sys_sem_signal()
 * Inputs:
 *      sys_mutex_t mutex           -- mutex to unlock
 *---------------------------------------------------------------------------*/
void sys_mutex_unlock(sys_mutex_t *mutex)
{
    sys_sem_signal((sys_sem_t *)mutex);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Creates a new mailbox
 * Inputs:
        sys_mbox_t q            -- mailbox to create
 *      int size                -- Size of elements in the mailbox
 * Outputs:
 *      err_t                   -- ERR_MEM: create mailbox failed
                                   ERR_OK:  create mailbox success
 *---------------------------------------------------------------------------*/
err_t sys_mbox_new( sys_mbox_t *q, int size )
{
    err_t ret = ERR_MEM;

    /* parameter check */
    LWIP_ASSERT("q != NULL", q != NULL);
    LWIP_ASSERT("size != NULL", size != NULL);

    if(LOS_QueueCreate("", size, q, 0, sizeof(void *)) == LOS_OK)
    {
        ret = ERR_OK;
        //SYS_STATS_INC_USED(mbox);
    }
    
    return ret;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_free
 *---------------------------------------------------------------------------*
 * Description:
 *      Deallocates a mailbox. If there are messages still present in the
 *      mailbox when the mailbox is deallocated, it is an indication of a
 *      programming error in lwIP and the developer should be notified.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 * Outputs:
 *      sys_mbox_t              -- Handle to new mailbox
 *---------------------------------------------------------------------------*/
void sys_mbox_free( sys_mbox_t *q )
{
    /* parameter check */
    LWIP_ASSERT("q != NULL", q != NULL);
    
    LOS_QueueDelete(*q);
//    while (LOS_QueueDelete(*q) != LOS_OK)
//    {
//        LOS_TaskDelay(1);
//    }
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_post
 *---------------------------------------------------------------------------*
 * Description:
 *      Post the "msg" to the mailbox.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void *data              -- Pointer to data to post
 *---------------------------------------------------------------------------*/
void sys_mbox_post(sys_mbox_t *q, void *data)
{
    /* parameter check */
    LWIP_ASSERT("q != NULL", q != NULL);

    LOS_QueueWrite(*q, data, sizeof(void *), LOS_WAIT_FOREVER);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_trypost
 *---------------------------------------------------------------------------*
 * Description:
 *      Try to post the "msg" to the mailbox.  Returns immediately with
 *      error if cannot.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void *msg               -- Pointer to data to post
 * Outputs:
 *      err_t                   -- ERR_OK if message posted, else ERR_MEM
 *                                  if not.
 *---------------------------------------------------------------------------*/
err_t sys_mbox_trypost(sys_mbox_t *q, void *data)
{
    err_t ret = ERR_OK;

    /* parameter check */
    LWIP_ASSERT("q != NULL", q != NULL);

    if(LOS_QueueWrite(*q, data, sizeof(void *), LOS_NO_WAIT) != LOS_OK)
    {
        /* The queue was already full. */
        ret = ERR_MEM;
        SYS_STATS_INC( mbox.err );
    }

    return ret;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_mbox_fetch
 *---------------------------------------------------------------------------*
 * Description:
 *      Blocks the thread until a message arrives in the mailbox, but does
 *      not block the thread longer than "timeout" milliseconds (similar to
 *      the sys_arch_sem_wait() function). The "msg" argument is a result
 *      parameter that is set by the function (i.e., by doing "*msg =
 *      ptr"). The "msg" parameter maybe NULL to indicate that the message
 *      should be dropped.
 *
 *      The return values are the same as for the sys_arch_sem_wait() function:
 *      Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
 *      timeout.
 *
 *      Note that a function with a similar name, sys_mbox_fetch(), is
 *      implemented by lwIP.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void **msg              -- Pointer to pointer to msg received
 *      u32_t timeout           -- Number of milliseconds until timeout
 * Outputs:
 *      u32_t                   -- SYS_ARCH_TIMEOUT if timeout, else number
 *                                  of milliseconds until received.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_mbox_fetch( sys_mbox_t *q, void **data, u32_t timeout )
{
    u32_t ret;
    UINT64 time_start, time_end, time_diff;

    /* parameter check */
    LWIP_ASSERT("q != NULL", q != NULL);

    time_start = LOS_TickCountGet();
    
    if(timeout != 0UL)
    {
        if(LOS_QueueRead(*q, data, sizeof(void *), timeout) == LOS_OK)
        {
            time_end = LOS_TickCountGet();
            time_diff = time_end - time_start;
            ret = time_diff;
        }
        else
        {
            ret = SYS_ARCH_TIMEOUT;
        }
    }
    else
    {
        LOS_QueueRead(*q, data, sizeof(void *), LOS_WAIT_FOREVER);
        time_end = LOS_TickCountGet();
        time_diff = time_end - time_start;
        ret = time_diff;
    }

    return ret;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_mbox_tryfetch
 *---------------------------------------------------------------------------*
 * Description:
 *      Similar to sys_arch_mbox_fetch, but if message is not ready
 *      immediately, we'll return with SYS_MBOX_EMPTY.  On success, 0 is
 *      returned.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void **msg              -- Pointer to pointer to msg received
 * Outputs:
 *      u32_t                   -- SYS_MBOX_EMPTY if no messages.  Otherwise,
 *                                  return ERR_OK.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_mbox_tryfetch( sys_mbox_t *q, void **data )
{
    u32_t ret;

    /* parameter check */
    LWIP_ASSERT("q != NULL", q != NULL);
    LWIP_ASSERT("data != NULL", data != NULL);

    if(LOS_QueueRead(*q, data, sizeof(void *), 0UL) == LOS_OK)
    {
        ret = ERR_OK;
    }
    else
    {
        ret = SYS_MBOX_EMPTY;
    }

    return ret;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_thread_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Starts a new thread with priority "prio" that will begin its
 *      execution in the function "thread()". The "arg" argument will be
 *      passed as an argument to the thread() function. The id of the new
 *      thread is returned. Both the id and the priority are system
 *      dependent.
 * Inputs:
 *      char *name              -- Name of thread
 *      lwip_thread_fn function -- Pointer to function to run.
 *      void *arg               -- Argument passed into function
 *      int stacksize           -- Required stack amount in bytes
 *      int prio                -- Thread priority
 * Outputs:
 *      sys_thread_t            -- Pointer to per-thread timeouts.
 *---------------------------------------------------------------------------*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
    UINT32 task_id = 0;
    TSK_INIT_PARAM_S stInitParam;

    stInitParam.pfnTaskEntry    = (TSK_ENTRY_FUNC)function;
    stInitParam.usTaskPrio      = prio;
    stInitParam.pcName          = (CHAR *)name;
    stInitParam.auwArgs[0]      = (UINT32)arg;
    stInitParam.uwStackSize     = stacksize;
    stInitParam.uwResved        = LOS_TASK_STATUS_DETACHED;

    if (LOS_TaskCreate(&task_id, &stInitParam) != LOS_OK)
    {
        return 0;
    }

    return task_id;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_protect
 *---------------------------------------------------------------------------*
 * Description:
 *      This optional function does a "fast" critical region protection and
 *      returns the previous protection level. This function is only called
 *      during very short critical regions. An embedded system which supports
 *      ISR-based drivers might want to implement this function by disabling
 *      interrupts. Task-based systems might want to implement this by using
 *      a mutex or disabling tasking. This function should support recursive
 *      calls from the same task or interrupt. In other words,
 *      sys_arch_protect() could be called while already protected. In
 *      that case the return value indicates that it is already protected.
 *
 *      sys_arch_protect() is only required if your port is supporting an
 *      operating system.
 * Outputs:
 *      sys_prot_t              -- Previous protection level (not used here)
 *---------------------------------------------------------------------------*/
sys_prot_t sys_arch_protect( void )
{
    return (sys_prot_t)LOS_IntLock();
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_unprotect
 *---------------------------------------------------------------------------*
 * Description:
 *      This optional function does a "fast" set of critical region
 *      protection to the value specified by pval. See the documentation for
 *      sys_arch_protect() for more information. This function is only
 *      required if your port is supporting an operating system.
 * Inputs:
 *      sys_prot_t              -- Previous protection level (not used here)
 *---------------------------------------------------------------------------*/
void sys_arch_unprotect(sys_prot_t value)
{
    LOS_IntRestore(value);
}

#endif /* !NO_SYS */
