// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_openssl.h"
#include "azure_c_shared_utility/xlogging.h"
#include "lwip/apps/sntp.h"
//#include "lwip/apps/sntp_time.h"
#include "lwip/inet.h"
#include <time.h>

extern UINT32 LOS_MemInit(VOID *pPool, UINT32 uwSize);

#if 0
#define MEM2_MAX_SIZE (4*1024*1024)
__align(8) UINT8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0xD0000000)));
#endif
#if 1
#define MEM2_MAX_SIZE (64*1024)
__align(8) UINT8 mem2base[MEM2_MAX_SIZE];
#endif

UINT32 platform_pool_init(void)
{
    return LOS_MemInit(mem2base, MEM2_MAX_SIZE);
}

void *platform_malloc(UINT32 uwSize)
{
    return LOS_MemAlloc(mem2base, uwSize);
}
void *platform_realloc(void *pPtr, UINT32 uwSize)
{
    return LOS_MemRealloc(mem2base, pPtr, uwSize);
}
void* platform_calloc(size_t n, size_t size)
{
    return platform_malloc(n * size);
}
void platform_free(void *pPtr)
{
    (void) LOS_MemFree(mem2base, pPtr);
}
void platform_mem_test(void)
{
    UINT8 *p=0;

    p = platform_malloc(1024);
    sprintf((char*)p,"Memory Malloc %f", 123.0);
    printf("%s", p);
    platform_free(p);
}

int platform_init(void)
{
    #if 0
    ip_addr_t ipaddr;
    ipaddr.addr = inet_addr("120.25.108.11");
    sntp_setserver(0,&ipaddr);
    #endif
    sntp_init();
    #if 1
    UINT32 ts = 0;
    while(ts == 0){
        (void)LOS_TaskDelay(LOS_MS2Tick(2000));
        ts = get_time(0);
        printf("platform_init timestamp=%lu\r\n", ts);
    }
    #endif
    return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_openssl_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    return STRING_construct("(native; liteos; undefined)");
}

void platform_deinit(void)
{
    sntp_stop();
    return;
}
