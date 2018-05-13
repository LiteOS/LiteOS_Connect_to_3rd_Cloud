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
 //============================================================================
 //叶帆科技  2018-05-01 改写
 //============================================================================
#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "includes.h"
#include "arch/sys_arch.h"

#include "cmsis_os.h"
#include "los_sem.h"

#define SYS_THREAD_MAX 6
static u16_t s_nextthread = 0;

/*----------------------------------------------------------------------------*/
/*                      DEFINITIONS                                           */
/*----------------------------------------------------------------------------*/
#define LWIP_ARCH_TICK_PER_MS       1000/OS_CFG_TICK_RATE_HZ

/*----------------------------------------------------------------------------*/
/*                      VARIABLES                                             */
/*----------------------------------------------------------------------------*/

u32_t sys_now()
{
  return LOS_TickCountGet();
}

//yefan 2018-05-02  重写邮箱
//=====================================================================================
#if (CMSIS_OS_VER == 1)
//=====================================================================================
/*-----------------------------------------------------------------------------------*/
//Creates an empty mailbox.
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
		
#if SYS_STATS
      ++lwip_stats.sys.mbox.used;
    if(lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used){
        lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
    }
#endif /* SYS_STATS */
    if (*mbox == NULL){
        return ERR_MEM;
    }    
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
		if(osOK != osMessageDelete(*mbox))
		{
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		}
#if SYS_STATS
    --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
		while (osOK != osMessagePut(*mbox, (uint32_t)data, portMAX_DELAY))
		{
		}
}

/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    err_t result;
		osStatus ret;
		ret = osMessagePut(*mbox, (uint32_t)msg, 0);
		if( osOK == ret)
		{
			result = ERR_OK;
		}
		else
		{
			result = ERR_MEM;
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		}
    return result;
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
    else
		{
				ret = osMessageGet(*mbox, portMAX_DELAY);
				while(osEventMessage != ret.status)
				{
					ret = osMessageGet(*mbox, portMAX_DELAY);
				} // time is arbitrary
				*msg = (void *)ret.value.v;
				EndTime = osKernelSysTick();
        Elapsed = EndTime - StartTime;
        
        return ( Elapsed ); // return time blocked TODO test	
    }
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummyptr;
		osEvent ret;
	
    if( msg == NULL )
		{
        msg = &dummyptr;
    }
		ret = osMessageGet(*mbox, 0);
		if ( osEventMessage == ret.status )
		{
				*msg = (void *)ret.value.v;
        return ERR_OK;
    }
    else
		{
        return SYS_MBOX_EMPTY;
    }
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

    if ( s_nextthread < SYS_THREAD_MAX )
		{
				thread_def.name = (char *)name;
				thread_def.stacksize = stacksize;
				thread_def.tpriority = get_prio(prio);
				thread_def.pthread = (os_pthread)thread;
				id = osThreadCreate(&thread_def, arg);
				if (NULL != id)
				{
						return id;
        }
        else
				{
            return NULL;
        }
    }
    else
		{
        return NULL;
    }
}

//=====================================================================================
#elif (CMSIS_OS_VER == 2)
//=====================================================================================
/*-----------------------------------------------------------------------------------*/
//Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    osMessageQueueAttr_t queue_def;
    (void ) size;
		*mbox = osMessageQueueNew(MAX_QUEUES, MAX_QUEUE_ENTRIES,&queue_def);
		
#if SYS_STATS
      ++lwip_stats.sys.mbox.used;
    if(lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used){
        lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
    }
#endif /* SYS_STATS */
    if (*mbox == NULL){
        return ERR_MEM;
    }
    
    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{	
		if(osOK != osMessageQueueDelete(*mbox))
		{
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		}
#if SYS_STATS
    --lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
		while (osOK != osMessageQueuePut(*mbox, data,0,portMAX_DELAY))
		{
		}
}

/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    err_t result;
		osStatus_t ret;
		ret = osMessageQueuePut(*mbox, msg, 0,0);
		if( osOK == ret)
		{
			result = ERR_OK;
		}
		else
		{
			result = ERR_MEM;
#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		}
    return result;
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
    UINT64 StartTime, EndTime, Elapsed;
		osStatus_t ret;
	
		StartTime = LOS_TickCountGet();
	
    if( msg == NULL ){
        msg = &dummyptr;
    }
    
    if( timeout != 0 ){
			  ret = osMessageQueueGet(*mbox, *msg,NULL,timeout);		
				if ( osOK == ret ){
						EndTime = LOS_TickCountGet();
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
    else
		{
				ret = osMessageQueueGet(*mbox, *msg,NULL,portMAX_DELAY);	
				while(osOK == ret )
				{
					ret = osMessageQueueGet(*mbox, *msg,NULL,portMAX_DELAY);	
				} // time is arbitrary
				EndTime = LOS_TickCountGet();
        Elapsed = EndTime - StartTime;        
        return ( Elapsed ); // return time blocked TODO test	
    }
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummyptr;
		osStatus_t ret;
	
    if( msg == NULL )
		{
        msg = &dummyptr;
    }
		ret = osMessageQueueGet(*mbox, *msg,NULL,0);		
		if ( osOK == ret ){
        return ERR_OK;
    }
    else
		{
        return SYS_MBOX_EMPTY;
    }
}   


static osPriority_t get_prio(int prio)
{
		osPriority_t ret = osPriorityNormal;
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

/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread , void *arg, int stacksize, int prio)
{
		osThreadId_t id;
		osThreadAttr_t thread_def;

    if ( s_nextthread < SYS_THREAD_MAX )
		{
				thread_def.name = name;
		  	thread_def.priority = get_prio(prio);
	  		thread_def.stack_size = stacksize;				
			  id = osThreadNew(thread, arg, &thread_def);
				if (NULL != id)
				{
						return id;
        }
        else
				{
            return NULL;
        }
    }
    else
		{
        return NULL;
    }
}

//=====================================================================================
#endif
//=====================================================================================

/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)          
{      
   if (*mbox == SYS_MBOX_NULL){
        return 0;
    } 
    else{
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
		u32_t ret;
		ret = LOS_SemCreate(count,(UINT32 *)sem);
		if(ret != LOS_OK){
#if SYS_STATS
      ++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */	
        return ERR_MEM;
    }
    // Means it can't be taken
    if(count == 0){
				ret = LOS_SemPend((UINT32)*sem, 1);
    }

#if SYS_STATS
    ++lwip_stats.sys.sem.used;
    if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used){
        lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
    }
#endif /* SYS_STATS */

    return ERR_OK;
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
#if SYS_STATS
    --lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
		LOS_SemDelete((UINT32)*sem);
}
/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)                                               
{
    if (*sem == SYS_SEM_NULL){
        return 0;
    }    
    else{
        return 1; 
    }                                                 
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
    // keep track of how many threads have been created
    s_nextthread = 0;
}
/*-----------------------------------------------------------------------------------*/
                                      /* Mutexes*/
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
#if LWIP_COMPAT_MUTEX == 0
/* Create a new mutex*/
err_t sys_mutex_new(sys_mutex_t *mutex) 
{
		osMutexDef_t mutex_def;
    //*mutex = xSemaphoreCreateMutex();
		*mutex = osMutexCreate (&mutex_def);
    if(*mutex == NULL){
#if SYS_STATS
        ++lwip_stats.sys.mutex.err;
#endif /* SYS_STATS */
        return ERR_MEM;
    }

#if SYS_STATS
    ++lwip_stats.sys.mutex.used;
    if(lwip_stats.sys.mutex.max < lwip_stats.sys.mutex.used){
        lwip_stats.sys.mutex.max = lwip_stats.sys.mutex.used;
    }
#endif /* SYS_STATS */
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* Deallocate a mutex*/
void sys_mutex_free(sys_mutex_t *mutex)
{
#if SYS_STATS
    --lwip_stats.sys.mutex.used;
#endif /* SYS_STATS */
		osMutexDelete(*mutex);
}
/*-----------------------------------------------------------------------------------*/
/* Lock a mutex*/
void sys_mutex_lock(sys_mutex_t *mutex)
{
		osMutexWait(*mutex, portMAX_DELAY);
}

/*-----------------------------------------------------------------------------------*/
/* Unlock a mutex*/
void sys_mutex_unlock(sys_mutex_t *mutex)
{
		osMutexRelease(*mutex);
}
#endif /*LWIP_COMPAT_MUTEX*/

/*-----------------------------------------------------------------------------------*/
// TODO
/*-----------------------------------------------------------------------------------*/
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
  	sys_prot_t ret;
		ret = LOS_IntLock();
	  return ret;
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
		LOS_IntLock();
    for(;;);
}

