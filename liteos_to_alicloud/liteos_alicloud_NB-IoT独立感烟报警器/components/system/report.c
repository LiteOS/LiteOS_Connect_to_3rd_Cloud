/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include "report.h"
#include "iot_import.h"
#include "iot_export.h"

#include "lite-utils.h"
#include "utils_epoch_time.h"
#define HAL_Snprintf     snprintf


int iotx_midreport_reqid(char *requestId, char *product_key, char *device_name)
{
    int ret;
    /* requestId = pk+devicename+mid */
    ret = HAL_Snprintf(requestId,
                       MIDREPORT_REQID_LEN,
                       "%s_%s_mid",
                       product_key,
                       device_name);
    return ret;
}

int iotx_midreport_payload(char *msg, char *requestId, char *mid, char *pid)
{
    int ret;
    /*topic's json data: {"id":"requestId" ,"params":{"_sys_device_mid":mid,"_sys_device_pid":pid }}*/
    ret = HAL_Snprintf(msg,
                       MIDREPORT_PAYLOAD_LEN,
                       "{\"id\":\"%s\",\"params\":{\"_sys_device_mid\":\"%s\",\"_sys_device_pid\":\"%s\"}}",
                       requestId,
                       mid,
                       pid);
    return ret;
}

int iotx_midreport_topic(char *topic_name, char *topic_head, char *product_key, char *device_name)
{
    int ret;
    /* reported topic name: "/sys/${productKey}/${deviceName}/thing/status/update" */
    ret = HAL_Snprintf(topic_name,
                       IOTX_URI_MAX_LEN,
                       "%s/sys/%s/%s/thing/status/update",
                       topic_head,
                       product_key,
                       device_name);
    return ret;
}

#define REPORT_TS_LEN 16
#define REPORT_PARA_LEN 80

static void Get_timestamp_str(char *buf, int len)
{
    UINT64 ret = 0;
    int retry = 0;

    do {
        ret = utils_get_epoch_time_from_ntp(buf, len);
    } while (ret == 0 && ++retry < 10);

    if (retry > 1) {
        log_err("utils_get_epoch_time_from_ntp() retry = %d.", retry);
    }

    if (ret == 0) {
        log_err("utils_get_epoch_time_from_ntp() failed!");
    }

    return;
}

int iotx_propreport_payload_sdm200(char *msg, char smoke_state, char remove_state, char bat_per, int period)
{
    int     ret;
    char    para_smoke[REPORT_PARA_LEN] = {0};
    char    para_remove[REPORT_PARA_LEN] = {0};
    char    para_bat[REPORT_PARA_LEN] = {0};
    char    para_period[REPORT_PARA_LEN] = {0};
    char    time_stamp_str[REPORT_TS_LEN] = {0};

    Get_timestamp_str(time_stamp_str, sizeof(time_stamp_str));
    
    HAL_Snprintf(para_smoke,
                REPORT_PARA_LEN,
                "\"Smoke\":{\"value\":%d,\"time\":%s}",
                smoke_state,
                time_stamp_str);                
    HAL_Snprintf(para_remove,
                REPORT_PARA_LEN,
                "\"Remove\":{\"value\":%d,\"time\":%s}",
                remove_state,
                time_stamp_str); 
    HAL_Snprintf(para_bat,
                REPORT_PARA_LEN,
                "\"Bat\":{\"value\":%d,\"time\":%s}",
                bat_per,
                time_stamp_str); 
    HAL_Snprintf(para_period,
                REPORT_PARA_LEN,
                "\"Period\":{\"value\":%d,\"time\":%s}",
                period,
                time_stamp_str);           
    ret = HAL_Snprintf(msg,
                       REPORT_PAYLOAD_LEN,
                       "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{%s,%s,%s,%s},\"method\":\"thing.event.property.post\"}",
                       para_smoke,
                       para_remove,
                       para_bat,
                       para_period);
    
    return ret;
}

int iotx_propreport_topic(char *topic_name, char *topic_head, char *product_key, char *device_name)
{
    int ret;
    ret = HAL_Snprintf(topic_name,
                       IOTX_URI_MAX_LEN,
                       "%s/sys/%s/%s/thing/event/property/post",
                       topic_head,
                       product_key,
                       device_name);
    return ret;
}

