/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file timer.c
 * @brief Linux implementation of the timer interface.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"  
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "timer_platform.h"
#include "los_tick.h"
 
void sleep(int DelaySec)
{
// HAL_Delay(DelaySec*1000);
}  
  
bool has_timer_expired(Timer *timer) {
  //uint32_t tickstart = 0;
  uint64_t tickstart = 0;
  __disable_irq();

  //tickstart = HAL_GetTick();
  tickstart = LOS_TickCountGet();
  long left = timer->end_time - tickstart;

  __enable_irq();

  return (left < 0);
}

void countdown_ms(Timer *timer, uint32_t timeout) {
  //uint32_t tickstart = 0;
  uint64_t tickstart = 0;
  __disable_irq();

  //tickstart = HAL_GetTick();
  tickstart = LOS_TickCountGet();
  timer->end_time = tickstart + timeout;

  __enable_irq();
}

uint32_t left_ms(Timer *timer) {
  //uint32_t tickstart = 0;
  uint64_t tickstart = 0;
  __disable_irq();

  //tickstart = HAL_GetTick();
  tickstart = LOS_TickCountGet();
  long left = timer->end_time - tickstart;

  __enable_irq();

  return (left < 0) ? 0 : left;
}

void countdown_sec(Timer *timer, uint32_t timeout) {
  //uint32_t tickstart = 0;
  uint64_t tickstart = 0;
  __disable_irq();

  //tickstart = HAL_GetTick();
  tickstart = LOS_TickCountGet();
  timer->end_time = tickstart + (timeout * 1000);

  __enable_irq();
}

void init_timer(Timer *timer) {
  timer->end_time = 0;
}
void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
  //timer->init_tick = HAL_GetTick();
  timer->init_tick = LOS_TickCountGet();
  timer->end_time = timeout_ms;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
  TimerCountdownMS(timer, timeout * 1000);
}


int TimerLeftMS(Timer* timer)
{
  int ret = 0;
  //uint32_t cur_tick = HAL_GetTick();  // The HAL tick period is 1 millisecond.
  uint32_t cur_tick = LOS_TickCountGet();  // The HAL tick period is 1 millisecond.
  if (cur_tick < timer->init_tick)
  { // Timer wrap-around detected
    // printf("Timer: wrap-around detected from %d to %d\n", timer->init_tick, cur_tick);
    timer->end_time -= 0xFFFFFFFF - timer->init_tick;
    timer->init_tick = 0;
  }
  ret = timer->end_time - (cur_tick - timer->init_tick);

  return (ret >= 0) ? ret : 0;
}


char TimerIsExpired(Timer* timer)
{
  return (TimerLeftMS(timer) > 0) ? 0 : 1;
}


void TimerInit(Timer* timer)
{
  timer->init_tick = 0;
  timer->end_time = 0;
}

#ifdef __cplusplus
}
#endif
