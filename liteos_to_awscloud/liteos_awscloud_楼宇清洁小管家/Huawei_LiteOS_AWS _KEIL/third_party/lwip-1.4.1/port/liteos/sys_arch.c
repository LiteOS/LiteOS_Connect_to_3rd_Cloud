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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
//#include "FreeRTOS.h"
//#include "task.h"

#include "cmsis_os.h"
#include "los_sem.h"

//xTaskHandle xTaskGetCurrentTaskHandle( void ) PRIVILEGED_FUNCTION;

///* This is the number of threads that can be started with sys_thread_new() */
//#define SYS_THREAD_MAX 6

//static u16_t s_nextthread = 0;

#define LOS_TASK_STATUS_DETACHED                    0x0100
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

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{	
	err_t ret = ERR_MEM;

    /* parameter check */
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("size != NULL", size != NULL);

    if(LOS_QueueCreate("", size, mbox, 0, sizeof(void *)) == LOS_OK)
    {
        ret = ERR_OK;
        //SYS_STATS_INC_USED(mbox);
    }
    
    return ret;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
	 /* parameter check */
    LWIP_ASSERT("sem != NULL", mbox != NULL);
    
    // SYS_STATS_DEC(sem.used);
    //LOS_MuxDelete(*mbox);
	LOS_QueueDelete(*mbox);
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

    //LOS_MuxPost(*sem);
	LOS_SemPost(*sem);
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
	 /* parameter check */
    LWIP_ASSERT("mbox != NULL", mbox!= NULL);

	if(LOS_OK != LOS_QueueWrite(*mbox, data, sizeof(void *), LOS_WAIT_FOREVER)	)	
	{
		//printf("sys_mbox_post err\n");
	}else
	{
		//printf("sys_mbox_post OK\n");
	}


}


/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	err_t ret = ERR_OK;

    /* parameter check */
    LWIP_ASSERT("mbox != NULL", mbox != NULL);

    if(LOS_QueueWrite(*mbox, msg, sizeof(void *), LOS_NO_WAIT) != LOS_OK)
    {
        /* The queue was already full. */
        ret = ERR_MEM;
        SYS_STATS_INC( mbox.err );
    }

    return ret;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	u32_t ret;
    UINT64 time_start, time_end, time_diff;

    /* parameter check */
    LWIP_ASSERT("mbox != NULL", mbox != NULL);

    time_start = LOS_TickCountGet();
    
    if(timeout != 0UL)
    {
        if(LOS_QueueRead(*mbox, &(*msg), sizeof(void *), timeout) == LOS_OK)
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
        LOS_QueueRead(*mbox, &(*msg), sizeof(void *), LOS_WAIT_FOREVER);
        time_end = LOS_TickCountGet();
        time_diff = time_end - time_start;
        ret = time_diff;
    }

    return ret;
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	u32_t ret;

    /* parameter check */
    LWIP_ASSERT("q != NULL", mbox!= NULL);
    LWIP_ASSERT("data != NULL", msg != NULL);

    if(LOS_QueueRead(*mbox, &(*msg), sizeof(void *), 0UL) == LOS_OK)
    {
        ret = ERR_OK;
    }
    else
    {
        ret = SYS_MBOX_EMPTY;
    }

    return ret;
}
/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)       
{    
  	
  if (*mbox == SYS_MBOX_NULL)
  {	  
    return 0;
  }
  else
  {
    return 1;
  }
}                                             
/*-----------------------------------------------------------------------------------*/                                              
void sys_mbox_set_invalid(sys_mbox_t *mbox)   
{                                             
  *mbox = SYS_MBOX_NULL;                      
}                                             

/*-----------------------------------------------------------------------------------*/
//  Creates a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	 err_t ret = ERR_MEM;

    /* create mutex */
    if(LOS_SemCreate(count,sem)!= LOS_OK)
    {
        ret = ERR_MEM;
        //SYS_STATS_INC(sem.err);
    }
    else
    {
        ret = ERR_OK;
        //SYS_STATS_INC_USED(sem);
    }



    return ret;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	u32_t ret;
    UINT64 time_start, time_end, time_diff;

    /* parameter check */
    LWIP_ASSERT("sem != NULL", sem != NULL);

    time_start = LOS_TickCountGet();
    
    if( timeout != 0UL )
    {
        if(LOS_SemPend(*sem, timeout) == LOS_OK)
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
        while(LOS_OK != LOS_SemPend(*sem, LOS_WAIT_FOREVER))
		{
		}
        time_end = LOS_TickCountGet();
        time_diff = time_end - time_start;
        ret = time_diff;
    }

    return ret;
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
  /* parameter check */
    LWIP_ASSERT("sem != NULL", sem != NULL);
    
    // SYS_STATS_DEC(sem.used);
    LOS_SemDelete(*sem);
}
/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)                                               
{
  if (*sem == SYS_SEM_NULL)
    return 0;
  else
    return 1;                                       
}

/*-----------------------------------------------------------------------------------*/                                                                                                                                                                
void sys_sem_set_invalid(sys_sem_t *sem)                                        
{                                                                               
  *sem = SYS_SEM_NULL;                                                          
} 

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void sys_init(void)
{

}
/*-----------------------------------------------------------------------------------*/
                                      /* Mutexes*/
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
#if LWIP_COMPAT_MUTEX == 0
/* Create a new mutex*/
err_t sys_mutex_new(sys_mutex_t *mutex) {
	
    if(LOS_MuxCreate(mutex) != LOS_OK){
        return ERR_MEM;
    }
    return ERR_OK;

}
/*-----------------------------------------------------------------------------------*/
/* Deallocate a mutex*/
void sys_mutex_free(sys_mutex_t *mutex)
{
	LOS_MuxDelete(*mutex);
}
/*-----------------------------------------------------------------------------------*/
/* Lock a mutex*/
void sys_mutex_lock(sys_mutex_t *mutex)
{
	LOS_MuxPend(*mutex,LOS_WAIT_FOREVER);
}

/*-----------------------------------------------------------------------------------*/
/* Unlock a mutex*/
void sys_mutex_unlock(sys_mutex_t *mutex)
{
	LOS_MuxPost(*mutex);
}
#endif /*LWIP_COMPAT_MUTEX*/
/*-----------------------------------------------------------------------------------*/
// TODO
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread , void *arg, int stacksize, int prio)
{
	UINT32 task_id = 0;
    TSK_INIT_PARAM_S stInitParam;

    stInitParam.pfnTaskEntry    = (TSK_ENTRY_FUNC)thread;
    stInitParam.usTaskPrio      = prio;
    stInitParam.pcName          = (CHAR *)name;
    stInitParam.uwArg           = (UINT32)arg;
    stInitParam.uwStackSize     = stacksize;
    stInitParam.uwResved        = LOS_TASK_STATUS_DETACHED;

    if (LOS_TaskCreate(&task_id, &stInitParam) != LOS_OK)
    {
        return 0;
    }

    return task_id;
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t sys_arch_protect(void)
{
	return (sys_prot_t)LOS_IntLock();
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
	LOS_IntRestore(pval);
}

/*
 * Prints an assertion messages and aborts execution.
 */
void sys_assert( const char *msg )
{	
	( void ) msg;
	/*FSL:only needed for debugging
	printf(msg);
	printf("\n\r");
	*/
    //vPortEnterCritical(  );
	LOS_IntLock();
    for(;;)
    ;
}
