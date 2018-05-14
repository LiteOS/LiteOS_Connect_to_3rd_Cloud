/**
  ******************************************************************************
  * @file    net_tcp_lwip.c
  * @author  MCD Application Team
  * @brief   Network abstraction at transport layer level. TCP implementation on
             LwIP.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "net_internal.h"

#ifdef USE_LWIP
#include "lwip/netdb.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
int net_sock_create_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
int net_sock_open_lwip(net_sockhnd_t sockhnd, const char * hostname, int remoteport, int localport);
int net_sock_recv_tcp_lwip(net_sockhnd_t sockhnd, uint8_t * buf, size_t len);
int net_sock_recvfrom_udp_lwip(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len, net_ipaddr_t * remoteaddress, int * remoteport);
int net_sock_send_tcp_lwip( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
int net_sock_sendto_udp_lwip(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len,  net_ipaddr_t * remoteaddress, int remoteport);
int net_sock_close_tcp_lwip(net_sockhnd_t sockhnd);
int net_sock_destroy_tcp_lwip(net_sockhnd_t sockhnd);
int net_get_hostaddress_lwip(net_hnd_t nethnd, net_ipaddr_t * ipAddress, const char * host);

/* Functions Definition ------------------------------------------------------*/

int net_sock_create_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto)
{
  int rc = NET_ERR;
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  net_sock_ctxt_t *sock = NULL;
  
  sock = net_malloc(sizeof(net_sock_ctxt_t));
  if (sock == NULL)
  {
    msg_error("net_sock_create allocation failed.\n");
    rc = NET_ERR;
  }
  else
  {
    memset(sock, 0, sizeof(net_sock_ctxt_t));
    sock->net = ctxt;
    sock->next = ctxt->sock_list;
    sock->methods.open            = (net_sock_open_lwip);
    switch(proto)
    {
      case NET_PROTO_TCP:
        sock->methods.recv        = (net_sock_recv_tcp_lwip);
        sock->methods.send        = (net_sock_send_tcp_lwip);
        break;
      case NET_PROTO_UDP:
        sock->methods.recvfrom    = (net_sock_recvfrom_udp_lwip);
        sock->methods.sendto      = (net_sock_sendto_udp_lwip);
        break;
      default:
        free(sock);
        return NET_PARAM;
    }
    sock->methods.close           =  (net_sock_close_tcp_lwip);
    sock->methods.destroy         =  (net_sock_destroy_tcp_lwip);
    sock->proto             = proto;
    sock->blocking          = NET_DEFAULT_BLOCKING;
    sock->read_timeout      = NET_DEFAULT_BLOCKING_READ_TIMEOUT;
    sock->write_timeout     = NET_DEFAULT_BLOCKING_WRITE_TIMEOUT;
    ctxt->sock_list         = sock; /* Insert at the head of the list */
    *sockhnd = (net_sockhnd_t) sock;

    rc = NET_OK;
  }
  
  return rc;
}


int net_sock_open_lwip(net_sockhnd_t sockhnd, const char * hostname, int dstport, int localport)
{
  int rc = NET_OK;
  int ret = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  char portBuffer[6];
  struct addrinfo hints;
  struct addrinfo *list = NULL;
  struct addrinfo *current = NULL;
  int socket = -1;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;        // TODO: Add IP version to the create() function parameter list.

  switch (sock->proto)
  {
    case NET_PROTO_TCP:
      if (localport != 0)
      { /* TCP local port setting is not implemented */
        rc = NET_PARAM;
      }
      else
      {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(portBuffer, 5, "%d", dstport);
      }
      break;
    case NET_PROTO_UDP:
      if (dstport != 0)
      { /* UDP default remote port setting is not implemented */
        rc = NET_PARAM;
      }
      else
      {  
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_flags    = AI_PASSIVE;
        snprintf(portBuffer, 5, "%d", localport);
      }
      break;
    default:
      return NET_PARAM;
  }
  
  if (rc != NET_OK)
  {
    return rc;
  }
  
  if( ((ret = getaddrinfo(hostname, portBuffer, &hints, &list)) != 0) || (list == NULL) )
  {
    msg_info("The address of %s could not be resolved. Error: %d.\n", hostname, ret);
    rc = NET_NOT_FOUND;
  }
  else
  {
    for(current = list; (current != NULL) && (socket == -1); current = current->ai_next)
    {
      socket = (int) socket(current->ai_family, current->ai_socktype, current->ai_protocol);
      if(socket >= 0)
      {
        if ( (sock->read_timeout != 0) && sock->blocking )
        {
#if !LWIP_SO_RCVTIMEO || !LWIP_SO_RCVRCVTIMEO_NONSTANDARD
#error  lwipopt.h must define LWIP_SO_RCVTIMEO so that the TCP read timeout is supported.
#endif /* !LWIP_SO_RCVTIMEO */
          int opt = sock->read_timeout;
          if (0 != lwip_setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt)))
          {
            msg_error("Could not set the read timeout.\n");
            rc = NET_ERR;
          }
        }

        if ( (rc == NET_OK) && (sock->write_timeout != 0) && sock->blocking )
        {
#if !LWIP_SO_SNDTIMEO || !LWIP_SO_SNDRCVTIMEO_NONSTANDARD
#error  lwipopt.h must define LWIP_SO_SNDTIMEO so that the TCP write timeout is supported.
#endif /* !LWIP_SO_RCVTIMEO */
          int opt = sock->write_timeout;
          if (0 != lwip_setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, &opt, sizeof(opt)))
          {
            msg_error("Could not set the write timeout.\n");
            rc = NET_ERR;
          }
        }
        
        if (rc == NET_OK)
        {
          switch(sock->proto)
          {
            case NET_PROTO_TCP:
            {
              if (0 == (ret = connect(socket, current->ai_addr, (uint32_t)current->ai_addrlen)))
              {
                sock->underlying_sock_ctxt = (net_sockhnd_t) socket;
                rc = NET_OK;
              }
              else
              {
                msg_error("connect() failed with error: %d\n", errno);
              }
              break;
            }
            case NET_PROTO_UDP:
            {
              if (0 == (ret = connect(socket, current->ai_addr, (uint32_t)current->ai_addrlen)))
              {
                if (-1 != bind(socket, current->ai_addr, current->ai_addrlen))
                {
                  sock->underlying_sock_ctxt = (net_sockhnd_t) socket;
                  rc = NET_OK;
                }
                else
                {
                  msg_error("bind() failed to bind.\n");
                }
              }
              else
              {
                msg_error("connect() failed with error: %d\n", errno);
              }
              break;
            }
            default:
              return NET_PARAM;
          }
        }
        else
        {
          close(socket);
          socket = -1;
          rc = NET_NOT_FOUND;
        }
      }
    }
    freeaddrinfo(list);
  }  
 
  return rc;
}


int net_sock_recv_tcp_lwip(net_sockhnd_t sockhnd, uint8_t * buf, size_t len)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
    
  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else
  {
    int ret = -1;
    do
    {
      ret = recv((int)sock->underlying_sock_ctxt, buf, len, sock->blocking ? 0 : MSG_DONTWAIT);
      if( ret < 0 )
      {
        switch(errno)
        {
          case EWOULDBLOCK:
          case EINTR:
            /* Incomplete read. The caller should try again. */
            break;
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case ERR_TIMEOUT:
            if ((sock->read_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            break;
          default:
            rc = NET_ERR;
        }
      }
      else
      {
        rc = ret;
      }
    } while ( (sock->blocking == true) && (rc == 0) );
  }

  return rc;
}


int net_sock_recvfrom_udp_lwip(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len, net_ipaddr_t * remoteaddress, int * remoteport)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
    
  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else
  {
    int ret = -1;
    struct sockaddr from;
    socklen_t fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));

    ret = recvfrom((int)sock->underlying_sock_ctxt, buf, len, sock->blocking ? 0 : MSG_DONTWAIT, &from, &fromlen);
    if (ret == 0)
    {
      rc = NET_EOF;
    }
    else if (ret < 0)
    {
      switch(errno)
      {
        case EINTR:
        case EWOULDBLOCK:
        case ERR_TIMEOUT:
          if ((sock->read_timeout != 0) && sock->blocking)
          {
            rc = NET_TIMEOUT;
          }
          break;
        case EPIPE:
        case ECONNRESET:
          rc = NET_EOF;
          break;
        default:
          rc = NET_ERR;
      }
    }
    else
    {
      if (from.sa_family == AF_INET)
      {
        rc = ret;
        struct sockaddr_in *saddr = (struct sockaddr_in *) &from;
        remoteaddress->ipv = NET_IP_V4;
        memset(remoteaddress->ip, 0xFF, sizeof(remoteaddress->ip));
        memcpy(&remoteaddress->ip[12], &saddr->sin_addr, 4);
        *remoteport = ntohs(saddr->sin_port);
      }
      else
      {
        /* IPv6 not implemented. */
        rc = NET_ERR;
      }
    }
  }

  return rc;
}


int net_sock_send_tcp_lwip( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else
  {
    int ret = -1;
    do
    {
      ret = send((int)sock->underlying_sock_ctxt, buf, len, 0);
      if(ret < 0)
      {
        switch(ret)
        {
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case EINTR:
           /* Incomplete read. The caller should try again. */
            break;
          case ERR_TIMEOUT:
            if ((sock->write_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            break;
          default:
            rc = NET_ERR;
        }
      }
      else
      {
        rc = ret;
      }
    } while ( (sock->blocking == true) && (rc == 0) );
  }
  
  return rc;
}


int net_sock_sendto_udp_lwip(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len, net_ipaddr_t * remoteaddress, int remoteport)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else
  {
    int ret = -1;
    do
    {
      struct sockaddr_in to;
      memset(&to, 0, sizeof(to));

      to.sin_len = 4;
      to.sin_family = (remoteaddress->ipv == NET_IP_V4) ? AF_INET : AF_UNSPEC;
      to.sin_port = htons(remoteport);
      memcpy(&to.sin_addr.s_addr, &remoteaddress->ip[12], 4);
      
      ret = sendto((int)sock->underlying_sock_ctxt, buf, len, 0, (struct sockaddr *) &to, sizeof(to));
      if(ret < 0)
      {
        switch(ret)
        {
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case EINTR:
           /* Incomplete read. The caller should try again. */
            break;
          case ERR_TIMEOUT:
            if ((sock->write_timeout != 0) && sock->blocking)
            {
              rc = NET_TIMEOUT;
            }
            break;
          default:
            rc = NET_ERR;
        }
      }
      else
      {
        rc = ret;
      }
    } while ( (sock->blocking == true) && (rc == 0) );
  }
  
  return rc;
}


int net_sock_close_tcp_lwip(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  if(((int) sock->underlying_sock_ctxt) >= 0)
  {
    if(0 == shutdown((int)sock->underlying_sock_ctxt, SHUT_RDWR))
    {
      if (0 == close((int)sock->underlying_sock_ctxt))
      {
        sock->underlying_sock_ctxt = (net_sockhnd_t) -1;
        rc = NET_OK;
      }
      else
      {
        msg_error("Could not close the socket %d. Error: %d\n", (int)sock->underlying_sock_ctxt, errno);
      }
    }
    else
    {
      msg_error("Could not shutdown the socket %d. Error: %d - %s\n", (int)sock->underlying_sock_ctxt, errno, lwip_strerr(errno));
    }
  }
  else
  {
    msg_warning("Underlying socket already closed. Skipping net_sock_close_tcp_lwip().");
    rc = NET_OK;
  }
  
  return rc;
}


int net_sock_destroy_tcp_lwip(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  net_ctxt_t *ctxt = sock->net;
    
  /* Find the parent in the linked list.
   * Unlink and free.
   */
  if (sock == ctxt->sock_list)
  {
    ctxt->sock_list = sock->next;
    rc = NET_OK;
  }
  else
  {
    net_sock_ctxt_t *cur = ctxt->sock_list;
    do
    {
      if (cur->next == sock)
      {
        cur->next = sock->next;
        rc = NET_OK;
        break;
      }
      cur = cur->next;
    } while(cur->next != NULL);
  }
  if (rc == NET_OK)
  {
    net_free(sock);
  }
  
  return rc;
}


int net_get_hostaddress_lwip(net_hnd_t nethnd, net_ipaddr_t * ipAddress, const char * host)
{
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  int rc = NET_ERR;
  
  if ((ipAddress == NULL) || (host == NULL))
  {
    rc = NET_PARAM;
  }
  else
  {
    if (ctxt->lwip_netif.ip_addr.addr == 0)
    {
      /* The network interface is not configured. */
      rc = NET_PARAM;
    }
    else
    {
      int ret = 0;
      struct addrinfo hints;
      struct addrinfo *servinfo;

      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_flags = AI_PASSIVE;

      ret = getaddrinfo(host, NULL, &hints, &servinfo);
      if (ret != 0)
      {
          msg_error("getaddrinfo error: %d.\n", ret);
          rc = NET_ERR;
      }
      // servinfo now points to a linked list of 1 or more struct addrinfos
               
      ipAddress->ipv = NET_IP_V4;
      memset(ipAddress->ip, 0xFF, sizeof(ipAddress->ip));
      memcpy(&ipAddress->ip[12], &((struct sockaddr_in *)(servinfo->ai_addr))->sin_addr, 4);
      rc = NET_OK;
      freeaddrinfo(servinfo);
    }
  }  
  
  return rc;
}

#endif /* USE_LWIP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
