/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <los_memory.h>
#include <bsp_model_nbiot.h>

//#include <atparser.h>
#include <sal_arch.h>
#include <sal_ipaddr.h>
#include <sal.h>
#include "internal/sal_sockets_internal.h"
#include "bsp_model_nbiot.h"

#define TAG "bc95_nbiot_module"

#define HOST_ALIYUN_NTP_SERVER      ".aliyun.com"
#define DOMAIN_IP_NTP_SERVER_1      "182.92.12.11"
#define DOMAIN_IP_NTP_SERVER_2      "120.25.115.19"
#define DOMAIN_IP_NTP_SERVER_3      "230.107.6.88"
#define DOMAIN_IP_NTP_SERVER_4      "230.107.6.88"
#define DOMAIN_IP_NTP_SERVER_5      "182.92.12.11"
#define DOMAIN_IP_NTP_SERVER_6      "230.107.6.88"
#define DOMAIN_IP_NTP_SERVER_7      "230.107.6.88"

#define DOMAIN_IP_COAP "106.15.213.197"
//#define DOMAIN_IP "120.42.46.98"

#define BC95_MAX_LINK_NUM       6

/* Change to include data slink for each link id respectively. <TODO> */
typedef struct link_s {
    int fd;
    sal_sem_t sem_start;
    sal_sem_t sem_close;
} link_t;

static uint8_t inited = 0;
static link_t g_link[BC95_MAX_LINK_NUM];
static sal_mutex_t g_link_mutex;

static netconn_data_input_cb_t g_netconn_data_input_cb;

static int fd_to_linkid(int fd)
{
    int link_id;
    
    sal_mutex_lock(&g_link_mutex);

    for (link_id = 0; link_id < BC95_MAX_LINK_NUM; link_id++) {
        if (g_link[link_id].fd == fd) 
            break;
    }

    sal_mutex_unlock(&g_link_mutex);

    return link_id;
}

void bc95_nbiot_module_socket_data_handle(void)
{
    unsigned char ipaddr[16] = {0};

    unsigned int           len = 0;
    int           remoteport = 0;
    int           linkid = 0;
    char          *recvdata = NULL;
    
    //SAL_DEBUG("link %d get data from %s %s ,len %s \r\n", linkid, ipaddr, port, datalen);
    /* Prepare socket data */
    
    while (1) {
        if (g_netconn_data_input_cb && (g_link[linkid].fd >= 0)){
            recvdata = (char *)LOS_MemAlloc(OS_SYS_MEM_ADDR ,4096);
            if (!recvdata) {
                SAL_DEBUG("Error:out of memory.");
                return;
            }
            memset(recvdata, 0, 4096);
            len = model_iot_udp_recv(linkid, recvdata, 512, ipaddr, &remoteport);
            if (len) {
                if (g_netconn_data_input_cb(g_link[linkid].fd, recvdata, len, (char *)ipaddr, remoteport)) {
                    SAL_DEBUG("socket %d get data len %d fail to post to sal, drop it\n",g_link[linkid].fd, len);
                }
            
                SAL_DEBUG("socket data on link %d with length %d posted to sal\n", linkid, len);
            }
            LOS_MemFree(OS_SYS_MEM_ADDR, recvdata);
            linkid ++;
            if (linkid > BC95_MAX_LINK_NUM) {
                break;
            }
        } else {
            break;
        }
    }
    
    return;
}


static int bc95_nbiot_module_init(void)
{
    //int ret = 0;
    uint32_t linknum = 0;
    
    if (inited) {
        SAL_DEBUG("bc95 nbiot module have already inited \r\n");
        return 0;
    }
    
    memset(g_link, 0, sizeof(g_link));
    
    for(linknum = 0; linknum < BC95_MAX_LINK_NUM; linknum++){
        g_link[linknum].fd = -1;
    }

    if (model_init() != 0) {
         goto err;
    }
    if (model_iot_init() != 0) {
         goto err;
    }

    //at.oob(AT_CMD_DATA_RECV, NULL, 0, bc95_nbiot_module_socket_data_handle, NULL);

    inited = 1;
    
    return 0;
err:
    return -1;
}

static int bc95_nbiot_module_deinit()
{
    if(!inited){
        return 0;
    }
    inited = 0;
    return 0;
}

static int bc95_nbiot_module_domain_to_ip(char *domain, char ip[16])
{
    //char *pccmd = NULL;
    //char *head = NULL;
    //char *end = NULL;
    char *p_no = NULL;
    
    if (!inited){
        SAL_DEBUG("%s bc95 nbiot module haven't init yet \r\n", __func__);
        return -1;
    }
    
    if (NULL == domain || NULL == ip){
        SAL_DEBUG("invalid input at %s \r\n", __func__);
        return -1;
    }
    /*TODO wait for reponse for ever for now*/
    //aos_sem_wait(&g_domain_sem, AOS_WAIT_FOREVER);
    /*
     * formate is :
       +CDNSGIP: 1,"www.baidu.com","183.232.231.173","183.232.231.172"
       or :
       +CDNSGIP: 0,8
    */
    //if (strstr(IOTX_PRE_DTLS_SERVER_URI, domain) != NULL) {
    //}

    if ((p_no = strstr( domain, HOST_ALIYUN_NTP_SERVER)) != NULL) {
        switch (*(p_no - 1)) {
            case '1':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_1, sizeof(DOMAIN_IP_NTP_SERVER_1));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_1)] = '\0';
                break;
            case '2':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_2, sizeof(DOMAIN_IP_NTP_SERVER_2));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_2)] = '\0';
                break;
            case '3':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_3, sizeof(DOMAIN_IP_NTP_SERVER_3));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_3)] = '\0';
                break;
            case '4':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_4, sizeof(DOMAIN_IP_NTP_SERVER_4));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_4)] = '\0';
                break;
            case '5':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_5, sizeof(DOMAIN_IP_NTP_SERVER_5));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_5)] = '\0';
                break;
            case '6':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_6, sizeof(DOMAIN_IP_NTP_SERVER_6));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_6)] = '\0';
                break;
            case '7':
                memcpy(ip, DOMAIN_IP_NTP_SERVER_7, sizeof(DOMAIN_IP_NTP_SERVER_7));
                ip[sizeof(DOMAIN_IP_NTP_SERVER_7)] = '\0';
                break;
            default:
                goto err;
        }
    } else {
        /* We find a good IP, save it. */
        memcpy(ip, DOMAIN_IP_COAP, sizeof(DOMAIN_IP_COAP));
        ip[sizeof(DOMAIN_IP_COAP)] = '\0';
    }

    SAL_DEBUG("domain %s get ip %s \r\n", domain ,ip);
    return 0;
err:

    return -1;
}

static int bc95_nbiot_module_conn_start(sal_conn_t *conn)
{
    int  linkid = 0;
    //char *pccmd = NULL;
    
    if (!inited){
        SAL_DEBUG("%s bc95 nbiot module haven't init yet \r\n", __func__);
        return -1;
    }

    if (!conn || !conn->addr){
        SAL_DEBUG("%s %d - invalid input \r\n", __func__, __LINE__);
        return -1;
    }
#if 0
    /*if input addr is a domain, then turn it into ip addr */
    if (bc95_nbiot_module_domain_to_ip(conn->addr, ipaddr) != 0){
        if (strlen(conn->addr) >= sizeof(ipaddr)){
            SAL_DEBUG("%s invalid server addr %s \r\n", __func__, conn->addr);
            return -1;
        }
        strcpy(ipaddr, conn->addr);
    }
#endif
    sal_mutex_lock(&g_link_mutex);
    for (linkid = 0; linkid < BC95_MAX_LINK_NUM; linkid++){
        if (g_link[linkid].fd >= 0){
            continue;
        }
        g_link[linkid].fd = conn->fd;
        break;
    }
    sal_mutex_unlock(&g_link_mutex);

    if (linkid >= BC95_MAX_LINK_NUM) {
        SAL_DEBUG("No link available for now, %s failed. \r\n", __func__);
        return -1;
    }
    
    switch(conn->type){
        case TCP_SERVER:
        break;
        case TCP_CLIENT:
        break;
        case UDP_UNICAST:
            if (model_iot_udp_create(conn->l_port) < 0){
                SAL_DEBUG("udp create fail \r\n");
                goto err;
            }
        break;
        case SSL_CLIENT:
        case UDP_BROADCAST:
        default:
            SAL_DEBUG("bc95 nbiot module connect type %d not support \r\n", conn->type);
            goto err;
    }

    return 0;
err:

    sal_mutex_lock(&g_link_mutex);
    g_link[linkid].fd = -1;
    sal_mutex_unlock(&g_link_mutex);
    return -1;
}

static int bc95_nbiot_module_conn_close(int fd, int32_t remote_port)
{
    int  linkid = 0;
    int  ret = 0;

    if (!inited){
        SAL_DEBUG("%s bc95 nbiot module haven't init yet \r\n", __func__);
        return -1;
    }

    linkid = fd_to_linkid(fd);
    if (linkid >= BC95_MAX_LINK_NUM) {
        SAL_DEBUG("No connection found for fd (%d) in %s \r\n", fd, __func__);
        return -1;
    } 

    if (model_iot_udp_close(linkid) != 0){
        SAL_DEBUG("udp close fail\r\n");
        ret = -1;
    }

    sal_mutex_lock(&g_link_mutex);
    g_link[linkid].fd = -1;
    sal_mutex_unlock(&g_link_mutex);

    return ret;
}

static int bc95_nbiot_module_send(int fd,uint8_t *data,uint32_t len,
                         char remote_ip[16], int32_t remote_port, int32_t timeout)
{
    int  linkid;

    if (!inited){
        SAL_DEBUG("%s bc95 nbiot module haven't init yet \r\n", __func__);
        return -1;
    }

    linkid = fd_to_linkid(fd);
    if (linkid >= BC95_MAX_LINK_NUM) {
        SAL_DEBUG("No connection found for fd (%d) in %s \r\n", fd, __func__);
        return -1;
    }
    if (model_iot_udp_send(linkid, remote_ip, remote_port, data, len) != 0) {
        SAL_DEBUG("udp send failed  (%s %d)\r\n", __func__, __LINE__);
        return -1;
    }

    return 0;
}

static int bc95_nbiot_packet_input_cb_register(netconn_data_input_cb_t cb)
{
    if (cb)
        g_netconn_data_input_cb = cb;
    return 0;
}


sal_op_t bc95_sal_opt = {
    .version = "1.0.0",
    .init = bc95_nbiot_module_init,
    .start = bc95_nbiot_module_conn_start,
    .send = bc95_nbiot_module_send,
    .domain_to_ip = bc95_nbiot_module_domain_to_ip,
    .close = bc95_nbiot_module_conn_close,
    .deinit = bc95_nbiot_module_deinit,
    .register_netconn_data_input_cb = bc95_nbiot_packet_input_cb_register,
};


int bc95_sal_init(void)
{
    return sal_module_register(&bc95_sal_opt);
}
