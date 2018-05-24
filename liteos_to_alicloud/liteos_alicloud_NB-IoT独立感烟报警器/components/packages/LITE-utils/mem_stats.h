/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef __MEM_STATS_H__
#define __MEM_STATS_H__

#include "lite-utils_internal.h"

#if defined(_PLATFORM_IS_LINUX_)
    #include <execinfo.h>
#endif

typedef struct {
    void               *buf;
    int                 buflen;
    char               *func;
    int                 line;
#if defined(_PLATFORM_IS_LINUX_)
    char              **bt_symbols;
    int                 bt_level;
#endif
    LOS_DL_LIST         list;
} OS_malloc_record;

#endif  /* __MEM_STATS_H__ */

