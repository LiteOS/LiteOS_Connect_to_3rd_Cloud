// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#define WOLFSSL_OPTIONS_IGNORE_SYS
#include "wolfssl/options.h"
#include "wolfssl/ssl.h"
#include "wolfssl/error-ssl.h"

#if 1//def FREERTOS_ARCH_ESP8266
//#include "openssl/ssl.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#else
//mock header
#include "liteos_mock.h"
#include "azure_c_shared_utility/gballoc.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/tlsio_openssl.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/threadapi.h"

#define OPENSSL_FRAGMENT_SIZE 5120
#define OPENSSL_LOCAL_TCP_PORT 1000
#define MAX_RETRY 20
#define RETRY_DELAY 1000 * 1000 * 10 // 10s
#define RECEIVE_BUFFER_SIZE 1024
#define OPENSSL_SELECT_TIMEOUT 20

#define os_delay_us(x) ThreadAPI_Sleep((x)/1000)

struct timeval timeout = { OPENSSL_SELECT_TIMEOUT, 0 };

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    WOLFSSL* ssl;
    WOLFSSL_CTX* ssl_context;
    TLSIO_STATE tlsio_state;
    char* hostname;
    int port;
    char* certificate;
    const char* x509certificate;
    const char* x509privatekey;
    int sock;
    ip_addr_t target_ip;
} TLS_IO_INSTANCE;

/*this function destroys an option previously created*/
static void tlsio_openssl_DestroyOption(const char* name, const void* value)
{
    /*since all options for this layer are actually string copies, disposing of one is just calling free*/
    if (
        (name == NULL) || (value == NULL)
        )
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
    else
    {
        if (
            (strcmp(name, "TrustedCerts") == 0) ||
            (strcmp(name, "x509certificate") == 0) ||
            (strcmp(name, "x509privatekey") == 0)
        )
        {
            platform_free((void*)value);
        }
        else
        {
            LogError("not handled option : %s", name);
        }
    }
}


static OPTIONHANDLER_HANDLE tlsio_openssl_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle)
{
    (void)(tlsio_handle);
        
    /* Not implementing any options */
    return NULL;
}

static const IO_INTERFACE_DESCRIPTION tlsio_openssl_interface_description =
{
    tlsio_openssl_retrieveoptions,
    tlsio_openssl_create,
    tlsio_openssl_destroy,
    tlsio_openssl_open,
    tlsio_openssl_close,
    tlsio_openssl_send,
    tlsio_openssl_dowork,
    tlsio_openssl_setoption
};

static void indicate_open_complete(TLS_IO_INSTANCE* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete == NULL)
    {
        LogError("NULL on_io_open_complete.");
    }
    else
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}


static int lwip_net_errno(int fd)
{
    int sock_errno = 0;
    u32_t optlen = sizeof(sock_errno);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_errno, &optlen);
    return sock_errno;
}

static void lwip_set_non_block(int fd) 
{
  int flags = -1;
  int error = 0;

  while((void)1,1){
      flags = fcntl(fd, F_GETFL, 0);
      if (flags == -1){
          error = lwip_net_errno(fd);
          if (error != EINTR){
              break;
          }
      } else{
          break;
      }
  }

  fcntl(fd, F_SETFL, O_NONBLOCK);

}

static int openssl_thread_LWIP_CONNECTION(TLS_IO_INSTANCE* p)
{
    //LogInfo("openssl_thread_LWIP_CONNECTION begin: %d", system_get_free_heap_size());
    //system_show_malloc();

    int result = 0;
    int ret = 0;
    int ret_test = 0;
    int sock;

    struct sockaddr_in sock_addr;
    fd_set readset;
    fd_set writeset;
    fd_set errset;
    WOLFSSL_CTX *ctx;
    WOLFSSL *ssl;

    TLS_IO_INSTANCE* tls_io_instance = p;

    LogInfo("OpenSSL thread start...");

    {
        LogInfo("create SSL context");
        ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
        if (ctx == NULL) {
            result = __LINE__;
            LogError("create new SSL CTX failed");
        }
        else
        {
            tls_io_instance->ssl_context = ctx;
            LogInfo("set SSL context read buffer size");
            //wolfSSL_CTX_set_read_ahead(ctx, OPENSSL_FRAGMENT_SIZE);
            // LogInfo("create socket ......");
            // LogInfo("size before creating socket: %d", system_get_free_heap_size());
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                result = __LINE__;
                LogError("create socket failed");
            }
            else
            {
                int keepAlive = 1; //enable keepalive
                int keepIdle = 20; //20s
                int keepInterval = 2; //2s
                int keepCount = 3; //retry # of times

				tls_io_instance->sock = sock;
                LogInfo("sock: %d", sock);
                LogInfo("create socket OK");

                LogInfo("set socket keep-alive ");

                ret = 0;
                ret_test = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
                ret += ret_test?1:0;

                ret_test = setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
                ret += ret_test?1:0;

                ret_test = setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
                ret += ret_test?1:0;

                ret_test = setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
                ret += ret_test?1:0;
                
                if (ret != 0){
                    result = __LINE__;
                    LogError("set socket keep-alive failed, ret = %d ", ret);
                }
                else
                {
                    LogInfo("set socket keep-alive OK");

                    lwip_set_non_block(sock);

                    LogInfo("bind socket ......");
                    memset(&sock_addr, 0, sizeof(sock_addr));
                    sock_addr.sin_family = AF_INET;
                    sock_addr.sin_addr.s_addr = 0;
                    sock_addr.sin_port = 0; // random local port
                    ret = bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
                    
                    // LogInfo("bind return: %d", ret);
                    if (ret != 0) {
                        result = __LINE__;
                        LogError("bind socket failed");
                    }
                    else
                    {
                        LogInfo("bind socket OK");

                        memset(&sock_addr, 0, sizeof(sock_addr));
                        sock_addr.sin_family = AF_INET;
                        sock_addr.sin_addr.s_addr = tls_io_instance->target_ip.addr;
                        sock_addr.sin_port = (u16_t)htons(tls_io_instance->port);

                        ret = connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
                        //LogInfo("connect return: %d %s", ret, ip_ntoa(&tls_io_instance->target_ip));
                        //LogInfo("EINPROGRESS: %d", EINPROGRESS);
                        if (ret == -1) {
                            ret = lwip_net_errno(sock);
                            LogInfo("lwip_net_errno ret: %d", ret);
                            if (ret != 115) { // EINPROGRESS
                                result = __LINE__;
                                ret = -1;
                                LogError("socket connect failed, not EINPROGRESS %s", tls_io_instance->hostname);
                            }
                        }

                        if(ret != -1)
                        {
                            FD_ZERO(&readset);
                            FD_ZERO(&writeset);
                            FD_ZERO(&errset);

                            FD_SET(sock, &readset);
                            FD_SET(sock, &writeset);
                            FD_SET(sock, &errset);

                            ret = lwip_select(sock + 1, NULL, &writeset, &errset, NULL);
                            if (ret <= 0) {
                                result = __LINE__;
                                LogError("select failed: %d", lwip_net_errno(sock));
                            } else 
                            {
                                if (!FD_ISSET(sock, &writeset) || FD_ISSET(sock, &errset)) {
                                    result = __LINE__;
                                    LogError("socket Error: %d", lwip_net_errno(sock));
                                }else
                                {
                                    {
                                        LogInfo("Socket Connect OK");
                                        wolfSSL_CTX_set_verify(ctx,SSL_VERIFY_NONE,0);
                                        ssl = wolfSSL_new(ctx);
                                        //LogInfo("after ssl new");
                                        if (ssl == NULL) {
                                            result = __LINE__;
                                            LogError("create ssl failed");
                                        }
                                        else
                                        {

                                            tls_io_instance->ssl = ssl;
                                            // LogInfo("SSL set fd");
                                            ret = wolfSSL_set_fd(ssl, sock);
                                            LogInfo("SSL_set_fd ret:%d", ret);
                                            if (ret != 1){
                                                result = __LINE__;
                                                LogError("SSL_set_fd failed");
                                            }
                                            else{
                                                int retry_connect = 0;
                                                int connect_succeeded = false;

                                                LogInfo("SSL connect... ");

                                                FD_ZERO(&readset);
                                                FD_SET(sock, &readset);
                                                FD_ZERO(&writeset);
                                                FD_SET(sock, &writeset);
                                                FD_ZERO(&errset);
                                                FD_SET(sock, &errset);

                                                while (retry_connect < MAX_RETRY)
                                                {
                                                    int ssl_state;

                                                    ret = lwip_select(sock + 1, &readset, &writeset, &errset, &timeout);

                                                    if (ret == 0) {
                                                        result = __LINE__;
                                                        LogInfo("SSL connect timeout");
                                                        break;
                                                    }
                                                    if (FD_ISSET(sock, &errset)) {
														unsigned int len;
                                                        result = __LINE__;
                                                        LogInfo("error return : %d", lwip_net_errno(sock));
                                                        len = (unsigned int) sizeof( int );
                                                        if (0 != getsockopt (sock, SOL_SOCKET, SO_ERROR, &ret, &len))
                                                            LogInfo("SSL error ret : %d", ret);   // socket is in error state
                                                        break;
                                                    }

                                                    ret = wolfSSL_connect(ssl);
                                                    if (ret == 1) { // ssl connect success
                                                        connect_succeeded = true;
                                                        break;
                                                    }

                                                    FD_ZERO(&readset);
                                                    FD_ZERO(&writeset);
                                                    FD_ZERO(&errset);
                                                    FD_SET(sock, &errset);

                                                    ssl_state = wolfSSL_get_error(ssl, ret);
                                                    if (ssl_state == SSL_ERROR_WANT_READ) {
                                                        FD_SET(sock, &readset);
                                                    } else if(ssl_state == SSL_ERROR_WANT_WRITE) {
                                                        FD_SET(sock, &writeset);
                                                    } else {
                                                        LogInfo("SSL state:%d", ssl_state);
                                                        result = __LINE__;
                                                        break;
                                                    }

                                                    retry_connect = retry_connect + 1;
                                                    //LogInfo("SSL connect retry: %d", retry_connect);
                                                    os_delay_us(RETRY_DELAY);
                                                }

                                                if (connect_succeeded == false)
                                                {
                                                    result = __LINE__;
                                                    LogError("SSL_connect failed");
                                                }else{
                                                    LogInfo("SSL connect ok");
                                                    result = 0;
                                                }
                                            }
                                        }
                                    }
                                    
                                }
                            }
                        }
                    }

                }
            }
        
        }
    }
    //LogInfo("openssl_thread_LWIP_CONNECTION end: %d", system_get_free_heap_size());

    if(result!=0){
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
    }
    return result;
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result;
    unsigned char buffer[RECEIVE_BUFFER_SIZE];
    int rcv_bytes;

    rcv_bytes = wolfSSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
    // LogInfo("ssl_read: rcv_bytes %d", rcv_bytes);
    if (rcv_bytes >= 0)
    {
        result = 0;
        if (tls_io_instance->on_bytes_received == NULL)
        {
            LogError("NULL on_bytes_received.");
        }
        else
        {
            tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
        }
    }
    else{
        result = __LINE__;
    }
    
    return result;
}

static int destroy_openssl_instance(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    //LogInfo("destroy openssl begin: %d", system_get_free_heap_size());
    if (tls_io_instance != NULL)
    {
        if (tls_io_instance->ssl != NULL)
        {
            wolfSSL_free(tls_io_instance->ssl);
            tls_io_instance->ssl = NULL;
            LogInfo("SSL_free");
        }
        if (tls_io_instance->sock >= 0)
        {
            int close_ret = close(tls_io_instance->sock);
            if (close_ret != 0){
                result = __LINE__;;
                LogError("close socket failed");
            }
            LogInfo("close socket");
        }
        if (tls_io_instance->ssl_context != NULL)
        {
            wolfSSL_CTX_free(tls_io_instance->ssl_context);
            tls_io_instance->ssl_context = NULL;
            LogInfo("SSL_ctx_free");
        }
        
        //LogInfo("destroy end: %d", system_get_free_heap_size());
    }

    return result;
}

CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters)
{
    //LogInfo("tlsio_openssl_create begin: %d", system_get_free_heap_size());
    TLSIO_CONFIG* tls_io_config = (TLSIO_CONFIG*)io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tls_io_config == NULL)
    {
        result = NULL;
        LogError("NULL tls_io_config.");
    }
    else
    {
        result = (TLS_IO_INSTANCE*) platform_malloc(sizeof(TLS_IO_INSTANCE));

        if (result == NULL)
        {
            LogError("Failed allocating TLSIO instance.");
        }
        else
        {
            memset(result, 0, sizeof(TLS_IO_INSTANCE));
            mallocAndStrcpy_s(&result->hostname, tls_io_config->hostname);
            result->port = (int)tls_io_config->port;
            result->ssl_context = NULL;
            result->ssl = NULL;
            result->certificate = NULL;

            result->on_bytes_received = NULL;
            result->on_bytes_received_context = NULL;

            result->on_io_open_complete = NULL;
            result->on_io_open_complete_context = NULL;

            result->on_io_close_complete = NULL;
            result->on_io_close_complete_context = NULL;

            result->on_io_error = NULL;
            result->on_io_error_context = NULL;

            result->tlsio_state = TLSIO_STATE_NOT_OPEN;

            result->x509certificate = NULL;
            result->x509privatekey = NULL;
        }
    }

    return (CONCRETE_IO_HANDLE)result;
}

void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_OPENING) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            LogError("TLS destroyed with a SSL connection still active.");
        }
        if (tls_io_instance->certificate != NULL)
        {
            platform_free(tls_io_instance->certificate);
        }
        if (tls_io_instance->hostname != NULL)
        {
            platform_free(tls_io_instance->hostname);
        }
        if (tls_io_instance->x509certificate != NULL)
        {
            platform_free((void*)tls_io_instance->x509certificate);
        }
        if (tls_io_instance->x509privatekey != NULL)
        {
            platform_free((void*)tls_io_instance->x509privatekey);
        }
        platform_free(tls_io_instance);
    }
    //LogInfo("tlsio_openssl_destroy end: %d", system_get_free_heap_size());
}

int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    //LogInfo("tlsio_openssl_open begin: %d", system_get_free_heap_size());
    int result;

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {

        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN && tls_io_instance->tlsio_state != TLSIO_STATE_ERROR)
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            result = __LINE__;
            LogError("Invalid tlsio_state for open. Expected state is TLSIO_STATE_NOT_OPEN or TLSIO_STATE_ERROR.");
            if (tls_io_instance->on_io_error != NULL)
            {
                tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
            }
        }
        else
        {
            int netconn_retry = 0;
            int ret;

            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING;

            do {
                //LogInfo("size before netconn_gethostbyname: %d", system_get_free_heap_size());
                ret = netconn_gethostbyname(tls_io_instance->hostname, &tls_io_instance->target_ip);
            } while((ret != 0) && netconn_retry++ < MAX_RETRY);
            
            if (ret != 0 || openssl_thread_LWIP_CONNECTION(tls_io_instance) != 0){
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                indicate_open_complete(tls_io_instance, IO_OPEN_ERROR); 
                result = __LINE__;
                LogError("openssl_thread_LWIP_CONNECTION failed.");
                if (tls_io_instance->on_io_error != NULL)
                {
                    tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
                }
            }else{
                tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                indicate_open_complete(tls_io_instance, IO_OPEN_OK);    
                result = 0;
                os_delay_us(5000000); //delay added to give reconnect time to send last message
                //LogInfo("tlsio_openssl_open end: %d", system_get_free_heap_size());
            }
        }
    }
    return result;
}

int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result;

	//LogInfo("tlsio_openssl_close begin: %d", system_get_free_heap_size());
    LogInfo("tlsio_openssl_close");

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_OPENING))
        {
            result = __LINE__;
            tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
            LogError("Invalid tlsio_state for close. Expected state is TLSIO_STATE_OPEN or TLSIO_STATE_ERROR.");
        }
        else
        {
			int ret;

            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;

            ret = destroy_openssl_instance(tls_io_instance);
            if (ret != 0)
            {
                result = __LINE__;
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                if (tls_io_instance->on_io_error != NULL)
                {
                    tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
                }
            }
            else
            {
                
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                result = 0;
                if (tls_io_instance->on_io_close_complete != NULL)
                {
                    tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
                }   
            }
        }
    }
    //LogInfo("tlsio_openssl_close end: %d", system_get_free_heap_size());
    return result;
}
int tlsio_openssl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if ((tls_io == NULL) || (buffer == NULL) || (size == 0))
    {
        result = __LINE__;
        LogError("Invalid parameter.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __LINE__;
            LogError("TLS is not ready to send data. Expected state is TLSIO_STATE_OPEN.");
            //LogInfo("TLS is not ready to send data. Expected state is TLSIO_STATE_OPEN.");
        }
        else
        {
            int total_write = 0;
            int ret = 0;
            int need_sent_bytes = (int)size;

            fd_set writeset;
            fd_set errset;
            while(need_sent_bytes > 0)
            {
                FD_ZERO(&writeset);
                FD_SET(tls_io_instance->sock, &writeset);
                FD_ZERO(&errset);
                FD_SET(tls_io_instance->sock, &errset);

                ret = lwip_select(tls_io_instance->sock + 1, NULL, &writeset, &errset, &timeout);
                if (ret == 0) {
                    result = __LINE__;
                    LogError("select timeout and no data to be write");
                    break;
                } else if (ret < 0 || FD_ISSET(tls_io_instance->sock, &errset)) {
                    result = __LINE__;
                    LogError("get error %d", lwip_net_errno(tls_io_instance->sock));
                    break;
                }
                ret = wolfSSL_write(tls_io_instance->ssl, ((u8_t*)buffer)+total_write, (int)size);
                // LogInfo("SSL_write ret: %d", ret);
                //LogInfo("SSL_write res: %d, size: %d, retry: %d", res, size, retry);
                if(ret > 0){
                    total_write += ret;
                    need_sent_bytes = need_sent_bytes - ret;
                }
                else
                {
                    result = __LINE__;
                    LogError("SSL_write failed.");
                    break;
                }
            }

            if (need_sent_bytes != 0)
            {
                LogInfo("ssl write failed, return [-0x%x]", -ret);

                ret = wolfSSL_shutdown(tls_io_instance->ssl);
                LogInfo("wolfSSL_shutdown ret: %d", ret);

                result = __LINE__;
                if (on_send_complete != NULL)
                {
                    on_send_complete(callback_context, IO_SEND_ERROR);
                }
            }
            else
            {
                result = 0;
                // LogInfo("SSL Write OK");
                if (on_send_complete != NULL)
                {
                    on_send_complete(callback_context, IO_SEND_OK);
                }
            }

            // LogInfo("total write: %d", total_write);
        }
    }
    return result;
}

void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
        {
            int ret = decode_ssl_received_bytes(tls_io_instance);
            if (ret != 0)
            {
                ret = wolfSSL_shutdown(tls_io_instance->ssl);
                //LogInfo("wolfSSL_shutdown ret: %d", ret);
            }
        } 
        else
        {
            //LogError("Invalid tlsio_state for dowork. Expected state is TLSIO_STATE_OPEN.");
        }
    }

}

int tlsio_openssl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    (void)tls_io, (void)optionName, (void)value;

    /* Not implementing any options */
    return 0;
}

const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void)
{
    return &tlsio_openssl_interface_description;
}
