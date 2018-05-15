// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/gballoc.h"

#include <time.h>
#include "azure_c_shared_utility/agenttime.h"
#include "bsp_rtc.h"
#include "los_task.h"

static time_t system_time = 0;

void sntp_set_system_time(time_t t)
{
    LOS_TaskLock();
    system_time = t;
    LOS_TaskUnlock();
    printf("current time = %lu\r\n", system_time);
}

unsigned long
liteos_mktime(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

time_t get_time(time_t* p)
{
    time_t t;

    LOS_TaskLock();
    t = system_time;
    if (p)
    {
        *p = t;
    }
    LOS_TaskUnlock();

    return t;
}

struct tm* get_gmtime(time_t* currentTime)
{
    return 0;//gmtime(currentTime);
}

time_t get_mktime(struct tm* cal_time)
{
	return 0;// mktime(cal_time);
}

char* get_ctime(time_t* timeToGet)
{
    return 0;//ctime(timeToGet);
}

double get_difftime(time_t stopTime, time_t startTime)
{
    return (double)(stopTime - startTime);
}