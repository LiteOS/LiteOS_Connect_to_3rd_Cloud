;----------------------------------------------------------------------------
 ; Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 ; All rights reserved.
 ; Redistribution and use in source and binary forms, with or without modification,
 ; are permitted provided that the following conditions are met:
 ; 1. Redistributions of source code must retain the above copyright notice, this list of
 ; conditions and the following disclaimer.
 ; 2. Redistributions in binary form must reproduce the above copyright notice, this list
 ; of conditions and the following disclaimer in the documentation and/or other materials
 ; provided with the distribution.
 ; 3. Neither the name of the copyright holder nor the names of its contributors may be used
 ; to endorse or promote products derived from this software without specific prior written
 ; permission.
 ; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 ; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 ; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 ; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 ; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 ; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 ; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 ; WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 ; OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ; ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ;---------------------------------------------------------------------------*/
;----------------------------------------------------------------------------
 ; Notice of Export Control Law
 ; ===============================================
 ; Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 ; include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 ; Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 ; applicable export control laws and regulations.
 ;---------------------------------------------------------------------------*/

        PRESERVE8

        EXPORT  LOS_IntLock
        EXPORT  LOS_IntUnLock
        EXPORT  LOS_IntRestore
        EXPORT  LOS_StartToRun
        EXPORT  osTaskSchedule
        EXPORT  PendSV_Handler
        EXPORT  LOS_IntNumGet
        EXPORT  osDisableIRQ
        
        IMPORT  g_stLosTask
        IMPORT  g_pfnTskSwitchHook
        IMPORT  g_bTaskScheduled

OS_NVIC_INT_CTRL            EQU     0xE000ED04
OS_NVIC_SYSPRI2             EQU     0xE000ED20
OS_NVIC_PENDSV_PRI          EQU     0xF0F00000
OS_NVIC_PENDSVSET           EQU     0x10000000
OS_TASK_STATUS_RUNNING      EQU     0x0010

    AREA    |.text|, CODE, READONLY
    THUMB
    REQUIRE8

; LiteOS 通过该函数开启调度
LOS_StartToRun
    ; 设置systick、pendSV中断优先级: *((volatile uint32_t *)0xE000ED20) = 0xF0F00000;
    ; 设置systick、pendSV中断优先级为最低，是为了最大的保证其他中断的实时性
    LDR     R4, =OS_NVIC_SYSPRI2
    LDR     R5, =OS_NVIC_PENDSV_PRI
    STR     R5, [R4]

    ; 设置全局变量为1: g_bTaskScheduled = 1;
    LDR     R0, =g_bTaskScheduled
    MOV     R1, #1
    STR     R1, [R0]

    ; CONTROL 特殊寄存器置 2，表示使用PSP（线程栈）
    ; Cortex-M3 提供2个SP寄存器用于存放栈指针:MSP,PSP.(Main SP, Process SP), 可以把中断程序和用户程序的栈分开了
    MOV     R0, #2
    MSR     CONTROL, R0

    ; g_stLosTask.pstRunTask = g_stLosTask.pstNewTask;
    LDR     R0, =g_stLosTask
    LDR     R2, [R0, #4]
    LDR     R0, =g_stLosTask
    STR     R2, [R0]

    ; g_stLosTask.pstRunTask->usTaskStatus |= 0x0010;
    LDR     R3, =g_stLosTask
    LDR     R0, [R3]
    LDRH    R7, [R0 , #4]
    MOV     R8,  #OS_TASK_STATUS_RUNNING
    ORR     R7,  R8
    STRH    R7,  [R0 , #4]

    ; g_stLosTask.pstRunTask->pStackPointer += 36;
    LDR     R12, [R0]
    ADD     R12, R12, #36 ;#100

    ; 栈恢复，根据当前运行的栈指针，恢复寄存器数据到R0 R1 R2 R3 R12 LR PC xPSR
    LDMFD   R12!, {R0-R7}
    ;ADD     R12, R12, #72
    MSR     PSP, R12
    ;VPUSH   S0;
    ;VPOP    S0;

    MOV     LR, R5
    MSR     xPSR, R7

    ; 开启总中断, 跳到R6(PC)
    CPSIE   I
    BX      R6
    NOP
    ALIGN
    
    AREA KERNEL, CODE, READONLY
    THUMB
    
    ; 获取当前中断状态(IPSR 中断状态字寄存器)
LOS_IntNumGet
    MRS     R0, IPSR
    BX      LR

    ; 关闭总中断
osDisableIRQ
    CPSID   I
    BX      LR

    ; 获取当前总中断状态(PRIMASK 中断屏蔽寄存器), 然后关闭总中断
    ; 获取当前总中断状态是为了如果发生中断嵌套的话不会提前开启中断
LOS_IntLock
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

    ; 这个LiteOS没有用。
LOS_IntUnLock
    MRS     R0, PRIMASK
    CPSIE   I
    BX      LR

    ; 根据第一个参数来恢复总中断状态，跟LOS_IntLock配合使用
LOS_IntRestore
    MSR     PRIMASK, R0
    BX      LR

    ; 挂起一个pendSV中断，在pendSV中断中完成任务调度
    ; pendSV可以在所有中断处理完成之后再进入pendSV中断，可以有效的提高实时性
osTaskSchedule
    LDR     R0, =OS_NVIC_INT_CTRL
    LDR     R1, =OS_NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR

    ; 开始任务调度
PendSV_Handler
    ; 首先存储中断屏蔽寄存器，然后关闭总中断，之后恢复中断状态
    MRS     R12, PRIMASK
    CPSID   I

    ; 如果定义了g_pfnTskSwitchHook（系统调度的钩子函数）就调用，否则调用默认的系统调度函数TaskSwitch
    LDR     R2, =g_pfnTskSwitchHook
    LDR     R2, [R2]
    CBZ     R2, TaskSwitch
    PUSH    {LR};PUSH    {R12, LR}
    BLX     R2
    POP     {LR};POP     {R12, LR}

TaskSwitch
    ; PSP 此时存储的是用户程序的栈值
    MRS     R0, PSP

    ; 压栈： 把R4,R5,R6,R7,R8,R9,R10,R11,R12值分别压栈
    ; 分别对应的用户程序栈保存的值：R0,R1,R2,R3,R12,LR,PC,PSR
    STMFD   R0!, {R4-R12}
    ;VSTMDB  R0!, {D8-D15}

    ; 设置当前任务的栈地址
    ; g_stLosTask.pstRunTask->pStackPointer = PSP
    LDR     R5, =g_stLosTask
    LDR     R6, [R5]
    STR     R0, [R6]

    ; 清除当前任务的运行状态标识
    ; g_stLosTask.pstRunTask->usTaskStatus &= ~0x0010;
    LDRH    R7, [R6 , #4]
    MOV     R8,#OS_TASK_STATUS_RUNNING
    BIC     R7, R8 ;BIC     R7, R7, R8
    STRH    R7, [R6 , #4]

    ; g_stLosTask.pstRunTask = g_stLosTask.pstNewTask;
    LDR     R0, =g_stLosTask
    LDR     R0, [R0, #4]
    STR     R0, [R5]

    ; 设置当前任务的运行状态标识
    ; g_stLosTask.pstRunTask->usTaskStatus |= 0x0010;
    LDRH    R7, [R0 , #4]
    MOV     R8,  #OS_TASK_STATUS_RUNNING
    ORR     R7, R8;ORR     R7, R7, R8
    STRH    R7,  [R0 , #4]

    ; 把R4-R12的值存储到g_stLosTask.pstRunTask->pStackPointer指针指向的地址
    LDR     R1,   [R0]
    ;VLDMIA  R1!, {D8-D15}
    LDMFD   R1!, {R4-R12}
    MSR     PSP,  R1

    ;恢复中断屏蔽寄存器
    MSR     PRIMASK, R12
    BX      LR
    
    NOP
    ALIGN
    END
