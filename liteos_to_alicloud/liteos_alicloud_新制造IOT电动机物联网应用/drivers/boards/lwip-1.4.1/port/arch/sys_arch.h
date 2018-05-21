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
#ifndef __SYS_RTXC_H__
#define __SYS_RTXC_H__

//#include "includes.h"
#include "arch/cc.h"

//
#include "lwip/opt.h"
#include "lwip/err.h"
#include "los_sem.h"
#include "los_sem.ph"
#include "los_typedef.h"
#include "los_memory.h"


#ifdef SYS_ARCH_GLOBALS
#define SYS_ARCH_EXT
#else
#define SYS_ARCH_EXT extern
#endif

/*-----------------macros-----------------------------------------------------*/
#define LWIP_STK_SIZE	        512
#define LWIP_TASK_MAX	        8	
/* The user can change this priority level. 
 * It is important that there was no crossing with other levels.
 */
#define LWIP_TSK_PRIO         3
#define LWIP_TASK_START_PRIO  LWIP_TSK_PRIO
#define LWIP_TASK_END_PRIO    LWIP_TSK_PRIO +LWIP_TASK_MAX

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX        6

#define MAX_QUEUES        		10	// 消息邮箱的数量
#define MAX_QUEUE_ENTRIES 		20	// 每个消息邮箱的大小

#define SYS_MBOX_NULL         (void *)0
#define SYS_SEM_NULL          (void *)0

//#define sys_arch_mbox_tryfetch(mbox,msg)   sys_arch_mbox_fetch(mbox,msg,1)

//
typedef struct los_sem
{
    SEM_CB_S* sem;        /**< Semaphore attribute structure*/
} sem_t;

typedef struct los_sem sys_sem_t; // type of semiphores

struct sys_mbox
{
    int first, last;
    void** msgs;
    int mbox_size;
    int isFull;
    int isEmpty;
    unsigned int  not_empty;
    unsigned int  not_full;
    unsigned int  mutex;
};

typedef struct sys_mbox* sys_mbox_t; // type of mailboxes

struct sys_thread
{
    struct sys_thread* next;
    UINT32 pthread;
};

typedef unsigned int sys_thread_t;

//#define SYS_MBOX_NULL   0
#define SYS_SEM_NULL    (NULL)
//#define SYS_DEFAULT_THREAD_STACK_DEPTH	configMINIMAL_STACK_SIZE

#define sys_sem_valid(x)        (((*x).sem == NULL) ? 0 : 1)
#define sys_sem_set_invalid(x)  ( (*x).sem = NULL)

#define sys_arch_mbox_tryfetch(mbox,msg)   sys_arch_mbox_fetch(mbox,msg,1)
#define SYS_MBOX_NULL               (NULL)
#define sys_mbox_valid(mbox)  (*(mbox) != NULL)
#define sys_mbox_set_invalid(mbox) do { if((mbox) != NULL) { *(mbox) = NULL; }}while(0)

/** struct of LwIP mailbox */
//typedef u32_t      sys_sem_t;       // type of semiphores
//typedef u32_t      sys_mbox_t;      // type of mailboxes
//typedef u32_t      sys_thread_t;    // type of id of the new thread
typedef u32_t      sys_mutex_t;       // type of mutex
typedef u32_t      sys_prot_t;

#endif /* __SYS_RTXC_H__ */

