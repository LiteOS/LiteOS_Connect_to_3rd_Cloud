/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include <aos/aos.h>

#define  TAG  "SAL_DEVICE"
#define DEV_SAL_BC95

#ifdef DEV_SAL_BC95
extern int bc95_sal_init(void);
#endif

int sal_device_init()
{
    int ret = 0;

#ifdef DEV_SAL_BC95
    ret = bc95_sal_init();
#endif
    if (ret){
        //LOGE(TAG, "device init fail ret is %d\n", ret);
    }
    
    return ret;
}
