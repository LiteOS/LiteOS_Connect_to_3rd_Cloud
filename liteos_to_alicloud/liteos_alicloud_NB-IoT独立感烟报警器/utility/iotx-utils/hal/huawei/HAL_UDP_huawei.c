/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <aos/log.h>
//#include <aos/network.h>
//#include "coap_transport.h"
#include "iot_import_coap.h"
#include "bsp_model_nbiot.h"

#define TRANSPORT_ADDR_LEN 16


#define NETWORK_ADDR_LEN      (16)

/**@brief Create udp socket.
*
* @details Create udp socket.
*
* @retval COAP_SUCCESS   Indicates if udp socket was created successfully, else an an  error code
*                       indicating reason for failure.
*/
int HAL_UDP_create(void *p_socket)
{
    int sockfd = -1;

    if(NULL == p_socket){
        return -1;
    }

    if((sockfd = model_iot_udp_create(1)) < 0){
        return -1;
    }

    *(int *)p_socket  = sockfd;
    return 0;
}

 void HAL_UDP_close(void *p_socket)
{
    int socket_id = -1;

    if(NULL != p_socket){
        socket_id = *(int *)p_socket;
        model_iot_udp_close(socket_id);
    }
}

int HAL_UDP_write(void               *p_socket,
                       const coap_address_t    *p_remote,
                       const unsigned char    *p_data,
                       unsigned int            datalen)
{
    int rc = -1;

    if(NULL == p_socket) {
        return -1;
    }
   
    rc = model_iot_udp_send((char *)p_remote->addr, p_remote->port, (char *)p_data, datalen);
 
    //mote_addr.sin_port = htons(p_remote->port);
    // = sendto(socket_id, p_data, (size_t)datalen, 0,
    //          (const struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if(-1 == rc)
    {
        return -1;
    }
    return rc;

}


int HAL_UDP_read(void                *p_socket,
                              coap_address_t   *p_remote,
                              unsigned char   *p_data,
                              unsigned int     datalen)
{
    int socket_id = -1;
    int count = -1;

    if(NULL == p_remote || NULL == p_data || NULL == p_socket)
    {
        return -1;
    }

    socket_id = *(int *)p_socket;
    count = model_iot_udp_recv(socket_id, p_data, datalen, p_remote->addr, &p_remote->port);
    //count = recvfrom(socket_id, p_data, (size_t)datalen, 0, &from, (unsigned int*)&addrlen);
    if(-1 == count)
    {
        return -1;
    } 
    return count;
}

int HAL_UDP_readTimeout( void *p_socket,
                              coap_address_t *p_remote, unsigned char  *p_data,
                              unsigned int datalen,     unsigned int timeout )
{
    int count = -1;
    timeout = timeout / 1000;
    do {
        count = HAL_UDP_read(p_socket, p_remote, p_data, datalen);
    } while (timeout--);
    return count;
}


int HAL_UDP_resolveAddress(const char *p_host, char addr[NETWORK_ADDR_LEN])
{
    memcpy((char *)addr, "106.15.213.197", NETWORK_ADDR_LEN);
    return 0;
}

