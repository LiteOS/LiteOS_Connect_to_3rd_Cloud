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
//#include "esp_system.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "iot_import.h"

/** used for OS time - stNowTime */
#include "../modbus.h"

extern TDateTime stNowTime;

#define TAG  "MQTT"

#define ESP_LOGI(tag, format, ...)  printf(format, ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...)  printf(format, ##__VA_ARGS__)

int errno; /** @2018-05-15 在文件 arch.h 中也有 extern  */

static uint64_t _esp32_get_time_ms(void)
{
    uint64_t time_ms;
    
    /*(stNowTime.y-18) 暂不考虑 年 月 日 的转换 */
    time_ms = (stNowTime.h*3600 + stNowTime.min*60 + stNowTime.s) *1000 + stNowTime.ms;

    return time_ms;
}

static uint64_t _esp32_time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = -1;
    char service[6];

    memset(&hints, 0, sizeof(hints));

    ESP_LOGI(TAG, "establish tcp connection with server(host=%s port=%u) \r\n", host, port);

    hints.ai_family = AF_INET; //only IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        ESP_LOGE(TAG, "getaddrinfo error");
        return 0;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET) {
            ESP_LOGE(TAG, "socket type error \r\n");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);

        if (fd < 0) {
            ESP_LOGE(TAG, "create socket error.\r\n");
            rc = -1;
            continue;
        }

        ESP_LOGI(TAG, "\r\n\r\n Begin to connect...\r\n");    /** @2018-05-15 ADD Trace */        
        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;    /** @2018-05-15 正常连接成功时, fd 可能是在 socket() 返回 0, 所以其他代码中 rc 若是标识异常时请用 -1  */
            break;
        }

        close(fd);
        ESP_LOGE(TAG, "connect error \r\n");
        rc = -1;
    }

    if (-1 == rc) {
        ESP_LOGI(TAG, "fail to establish tcp \r\n");
    } else {
        ESP_LOGI(TAG, "success to establish tcp, fd=%d \r\n", rc);
    }

    freeaddrinfo(addrInfoList);

    return (uintptr_t)rc;
}


int HAL_TCP_Destroy(uintptr_t fd)
{
    int rc;

    //Shutdown both send and receive operations.
    rc = shutdown((int) fd, 2);

    if (0 != rc) {
        ESP_LOGE(TAG, "shutdown error \r\n");
        return -1;
    }

    rc = close((int) fd);

    if (0 != rc) {
        ESP_LOGE(TAG, "closesocket error \r\n");
        return -1;
    }

    return 0;
}


int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;

    t_end = _esp32_get_time_ms() + timeout_ms;
    len_sent = 0;
    //ret = 1; //send one time if timeout_ms is value 0  @2018-05-20  无需这样操作

    do {
        t_left = _esp32_time_left(t_end, _esp32_get_time_ms());

        /** @2018-05-16 t_left == 0 时不能 break, 否则此函数会 return 0 值 */
        /**  然后, 在函数 iotx_mc_decode_packet() 内会因为与期待读 len=1 & return 1 的情况相矛盾,从而认为 MQTT_NETWORK_ERROR */
        /**  所以, 对 t_left 做下面 2 行代码的特殊处理.   */
        if(t_left <= 0) 
            t_left = 2000;        

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &timeout);

            if (ret > 0) {
                if (0 == FD_ISSET(fd, &sets)) {
                    ESP_LOGI(TAG, "Should NOT arrive \r\n");
                    //If timeout in next loop, it will not sent any data
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                ESP_LOGI(TAG, "select-write timeout %d \r\n", (int)fd);
                break;
            } else {
            
                if (EINTR == errno) {
                    ESP_LOGI(TAG, "EINTR be caught \r\n");
                    continue;
              }

                ESP_LOGE(TAG, "select-write fail \r\n");
                break;
            }
        }

        if (ret > 0) {
            ret = send(fd, buf + len_sent, len - len_sent, 0);

            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                ESP_LOGI(TAG, "No data be sent \r\n");
            } else {
              if (EINTR == errno) {
                  ESP_LOGI(TAG, "EINTR be caught \r\n");
                  continue;
              }

                ESP_LOGE(TAG, "send fail \r\n");
                break;
            }
        }
    } while ((len_sent < len) && (_esp32_time_left(t_end, _esp32_get_time_ms()) > 0));

    return len_sent;
}


int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = _esp32_get_time_ms() + timeout_ms;
    len_recv = 0;
    err_code = 0;

    do {
        t_left = _esp32_time_left(t_end, _esp32_get_time_ms());

        if (0 == t_left) {            
            printf("\r\n  %s : %d (0 == t_left)\r\n", __func__, __LINE__); // @2018-05-16 t_left == 0 时不能 break, 否则此函数会 return 0 值
                                                                           //  然后, 在函数 iotx_mc_decode_packet() 内会因为与期待读 len=1 & return 1 的情况相矛盾,从而认为 MQTT_NETWORK_ERROR
//            break;            
        }

//        if(t_left <= 0) 
//            t_left = 2000;

        FD_ZERO(&sets);
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(fd + 1, &sets, NULL, NULL, &timeout);        
//        printf("\r\n%s %d select() ret : %d \r\n", __func__, __LINE__, ret );

        if (ret > 0) {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
//            printf("\r\n%s %d recv() ret : %d \r\n", __func__, __LINE__, ret );

            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                //ESP_LOGE(TAG, "connection is closed \r\n");
//                printf("\r\n%s %d connection is closed \r\n", __func__, __LINE__ );
                err_code = -1;
                break;
            } else {
                if (EINTR == errno) {
//                    printf("\r\n%s %d EINTR be caught \r\n", __func__, __LINE__ );
                    continue;
                }
//                printf("\r\n%s %d send fail \r\n", __func__, __LINE__ );
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
//            printf("\r\n  %s : %d  ret=recv(...) : %d\r\n", __func__, __LINE__, ret);
            break;
        } else {
//            printf("\r\n%s %d select-recv fail \r\n", __func__, __LINE__ );
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    //priority to return data bytes if any data be received from TCP connection.
    //It will get error code on next calling
    return (0 != len_recv) ? len_recv : err_code;
}
