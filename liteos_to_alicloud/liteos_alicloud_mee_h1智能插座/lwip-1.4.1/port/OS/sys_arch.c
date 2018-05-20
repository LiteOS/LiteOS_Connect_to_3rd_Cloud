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
#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "includes.h"
#include "arch/sys_arch.h"
#include "include.h"

/*----------------------------------------------------------------------------*/
/*                      DEFINITIONS                                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                      VARIABLES                                             */
/*----------------------------------------------------------------------------*/
u32_t sys_now()
{
    return LOS_TickCountGet();
}

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    osMessageQDef_t queue_def;
    osThreadId thread_id;

    (void ) size;
    (void ) thread_id;
    queue_def.queue_sz = MAX_QUEUES;
    queue_def.item_sz = sizeof( void * );
    queue_def.pool = NULL;
    *mbox = osMessageCreate(&queue_def, thread_id);

    return (*mbox == NULL) ? ERR_MEM : ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
    osMessageDelete(*mbox);
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
    while (osOK != osMessagePut(*mbox, (uint32_t)data, LOS_WAIT_FOREVER));
}


/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    return (osMessagePut(*mbox, (uint32_t)msg, 0)==osOK) ? ERR_OK : ERR_MEM;
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
    void *dummyptr;
    uint32_t StartTime, EndTime, Elapsed;
    osEvent ret;

    StartTime = osKernelSysTick();
	
    if( msg == NULL ){
        msg = &dummyptr;
    }
    
    if( timeout != 0 ){
				ret = osMessageGet(*mbox, timeout);
				if ( osEventMessage == ret.status ){
						*msg = (void *)ret.value.v;
						EndTime = osKernelSysTick();
            Elapsed = (EndTime - StartTime);
					
            return ( Elapsed );
        }
        // timed out blocking for message
        else{ 
            *msg = NULL;
            return SYS_ARCH_TIMEOUT;
        }
    } 
    // block forever for a message.
    else{
		ret = osMessageGet(*mbox, LOS_WAIT_FOREVER);
		while(osEventMessage != ret.status)
		{
			ret = osMessageGet(*mbox, LOS_WAIT_FOREVER);
		} // time is arbitrary
		*msg = (void *)ret.value.v;
		EndTime = osKernelSysTick();
        Elapsed = EndTime - StartTime;
        return ( Elapsed ); // return time blocked TODO test	
    }
}


/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)          
{      
    return (*mbox == SYS_MBOX_NULL) ? 0 : 1;
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
  return (LOS_SemCreate(count,(UINT32 *)sem)==0) ? 0 : -1;
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
    UINT64 StartTime, EndTime, Elapsed;
    u32_t ret;
    StartTime = LOS_TickCountGet();
	
    if(timeout != 0)
    {
        ret = LOS_SemPend((UINT32)*sem, timeout);
        if(LOS_OK == ret)
        {
            EndTime = LOS_TickCountGet();
            Elapsed = EndTime - StartTime;            
            return (Elapsed); // return time blocked TODO test	
        }
        else
        {
            return SYS_ARCH_TIMEOUT;
        }
    }
    // must block without a timeout
    else
    {
        while(LOS_OK != LOS_SemPend((UINT32)*sem, LOS_WAIT_FOREVER))
        {
        }
        EndTime = LOS_TickCountGet();
        Elapsed = EndTime - StartTime;
        return ( Elapsed ); // return time blocked  
    }    
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
	LOS_SemPost((UINT32)*sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
    LOS_SemDelete((UINT32)*sem);
}
/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)                                               
{
    return (*sem == SYS_SEM_NULL) ? 0 : 1;
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

static osPriority get_prio(int prio)
{
    osPriority ret = osPriorityNormal;
    if (prio >= 7)
    {
        ret = osPriorityRealtime;
    }
    else if(prio == 6)
    {
        ret = osPriorityHigh;
    }
    else if(prio == 5)
    {
        ret = osPriorityAboveNormal;
    }
    else if(prio == 4)
    {
        ret = osPriorityNormal;
    }
    else if(prio == 3)
    {
        ret = osPriorityBelowNormal;
    }
    else if(prio == 2)
    {
        ret = osPriorityLow;
    }
    else if(prio == 1)
    {
        ret = osPriorityIdle;
    }
    else
    {
        ret = osPriorityNormal;
    }
    return ret;
}

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
    osThreadId id;
    osThreadDef_t thread_def;

    thread_def.name = (char *)name;
    thread_def.stacksize = stacksize;
    thread_def.tpriority = get_prio(prio);
    thread_def.pthread = (os_pthread)thread;
    id = osThreadCreate(&thread_def, arg);

    return (NULL != id) ? id : NULL;
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
	return LOS_IntLock();
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
