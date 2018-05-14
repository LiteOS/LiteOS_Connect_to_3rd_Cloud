/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <timer_platform.h>
#include <network_interface.h>
#include "aws_iot_error.h"
#include "aws_iot_log.h"
#include "network_platform.h"
#include "heap.h"
#ifdef FIREWALL_MBEDLIB
#include "firewall_wrapper.h"
#endif /* FIREWALL_MBEDLIB */ 
#include "net.h"
#include "mbedtls_net.h"
#include "msg.h"

void _iot_tls_set_connect_params(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
                                 char *pDevicePrivateKeyLocation, char *pDestinationURL,
                                 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag);
/* This is the value used for ssl read timeout */
#define IOT_SSL_READ_TIMEOUT 10

extern int mbedtls_hardware_poll (void *data, unsigned char *output, size_t len, size_t *olen);

extern void *hrng;
extern void *hnet;

/*
 * This is a function to do further verification if needed on the cert received
 */

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
  ((void) level);
#if defined ( __GNUC__ ) && defined (HAS_RTOS)
  ((void) ctx);
  mbedtls_printf("%s:%04d: %s\n", file, line, str );
#else
  mbedtls_fprintf((FILE *) ctx, "%s:%04d: %s\n", file, line, str );
#endif
}


static int _iot_tls_verify_cert(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
  char buf[1024];
  ((void) data);

  msg_debug("\nVerify requested for (Depth %d):\n", depth);
  mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
  msg_debug("%s", buf);

  if((*flags) == 0)
  {
    msg_debug("  This certificate has no flags\n");
  }
  else
  {
    msg_debug(buf, sizeof(buf), "  ! ", *flags);
    msg_debug("%s\n", buf);
  }

  return 0;
}


void _iot_tls_set_connect_params(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
                                 char *pDevicePrivateKeyLocation, char *pDestinationURL,
                                 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag)
{
  pNetwork->tlsConnectParams.DestinationPort = destinationPort;
  pNetwork->tlsConnectParams.pDestinationURL = pDestinationURL;
  pNetwork->tlsConnectParams.pDeviceCertLocation = pDeviceCertLocation;
  pNetwork->tlsConnectParams.pDevicePrivateKeyLocation = pDevicePrivateKeyLocation;
  pNetwork->tlsConnectParams.pRootCALocation = pRootCALocation;
  pNetwork->tlsConnectParams.timeout_ms = timeout_ms;
  pNetwork->tlsConnectParams.ServerVerificationFlag = ServerVerificationFlag;
}

IoT_Error_t iot_tls_init(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
                         char *pDevicePrivateKeyLocation, char *pDestinationURL,
                         uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag)
{
  _iot_tls_set_connect_params(pNetwork, pRootCALocation, pDeviceCertLocation, pDevicePrivateKeyLocation,
                              pDestinationURL, destinationPort, timeout_ms, ServerVerificationFlag);

  pNetwork->connect = iot_tls_connect;
  pNetwork->read = iot_tls_read;
  pNetwork->write = iot_tls_write;
  pNetwork->disconnect = iot_tls_disconnect;
  pNetwork->isConnected = iot_tls_is_connected;
  pNetwork->destroy = iot_tls_destroy;

  pNetwork->tlsDataParams.flags = 0;

  return SUCCESS;
}

IoT_Error_t iot_tls_is_connected(Network *pNetwork)
{
  /* Use this to add implementation which can check for physical layer disconnect */
  return NETWORK_PHYSICAL_LAYER_CONNECTED;
}


/*
 * Amazon Profile
 */
const mbedtls_x509_crt_profile mbedtls_x509_crt_amazon_suite =
{
  /* Only SHA-256 and 384 */
  MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
  MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ),
  /* Only ECDSA */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSA) | /* */
  MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_ECKEY) | /* */
  MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_ECDSA ),
#if defined(MBEDTLS_ECP_C)
  /* Only NIST P-256 and P-384 */
  MBEDTLS_X509_ID_FLAG( MBEDTLS_ECP_DP_SECP256R1 ) |
  MBEDTLS_X509_ID_FLAG( MBEDTLS_ECP_DP_SECP384R1 ),
#else
  0,
#endif
  2048
};

IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *params)
{
  if(NULL == pNetwork)
  {
    return NULL_VALUE_ERROR;
  }

  if(NULL != params)
  {
    _iot_tls_set_connect_params(pNetwork, params->pRootCALocation, params->pDeviceCertLocation,
                                params->pDevicePrivateKeyLocation, params->pDestinationURL,
                                params->DestinationPort, params->timeout_ms, params->ServerVerificationFlag);
  }

  int ret = 0;
  const char *pers = "aws_iot_tls_wrapper";
#if 0 // 2k should be large enough! Not needed anyway.
#ifdef msg_debug
  unsigned char buf[MBEDTLS_SSL_MAX_CONTENT_LEN + 1];
#endif
#endif // 0
  TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);

  mbedtls_platform_set_calloc_free( heap_alloc , heap_free);
  mbedtls_ssl_config_init(&(tlsDataParams->conf));
  mbedtls_ctr_drbg_init(&(tlsDataParams->ctr_drbg));
  mbedtls_x509_crt_init(&(tlsDataParams->cacert));
  mbedtls_x509_crt_init(&(tlsDataParams->clicert));
  mbedtls_pk_init(&(tlsDataParams->pkey));
  mbedtls_debug_set_threshold(1);
  mbedtls_ssl_conf_dbg( &(tlsDataParams->conf), my_debug, stdout );


  msg_debug("\n  . Seeding the random number generator...");
  mbedtls_entropy_init(&(tlsDataParams->entropy));

  if( (ret = mbedtls_entropy_add_source( &(tlsDataParams->entropy), mbedtls_hardware_poll ,
                                         (void*)&hrng, 1, MBEDTLS_ENTROPY_SOURCE_STRONG) ) != 0 )
	//if( (ret = mbedtls_entropy_add_source( &(tlsDataParams->entropy), mbedtls_hardware_poll ,
  //                                       (void*)0, 1, MBEDTLS_ENTROPY_SOURCE_STRONG) ) != 0 )
  {
    mbedtls_printf( " failed\n  ! mbedtls_entropy_add_source returned %d\n", ret );
    return NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
  }



  if((ret = mbedtls_ctr_drbg_seed(&(tlsDataParams->ctr_drbg), mbedtls_entropy_func, &(tlsDataParams->entropy),
                                  (const unsigned char *) pers, strlen(pers))) != 0)
  {
    msg_error(" failed\n  ! mbedtls_ctr_drbg_seed returned -0x%x\n", -ret);
    return NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED;
  }



  msg_debug("  . Loading the CA root certificate ...");
  ret = mbedtls_x509_crt_parse(&(tlsDataParams->cacert), (unsigned char const *) pNetwork->tlsConnectParams.pRootCALocation, strlen(pNetwork->tlsConnectParams.pRootCALocation) + 1);
  if(ret < 0)
  {
    msg_error(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert\n\n", -ret);
    return NETWORK_X509_ROOT_CRT_PARSE_ERROR;
  }
  msg_debug(" ok (%d skipped)\n", ret);

  msg_debug("  . Loading the client cert. and key...");
  ret = mbedtls_x509_crt_parse(&(tlsDataParams->clicert), (unsigned char const *) pNetwork->tlsConnectParams.pDeviceCertLocation, strlen(pNetwork->tlsConnectParams.pDeviceCertLocation) + 1);
  if(ret != 0)
  {
    msg_error(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing device cert\n\n", -ret);
    return NETWORK_X509_DEVICE_CRT_PARSE_ERROR;
  }
#ifdef   FIREWALL_MBEDLIB
  ret = mbedtls_firewall_pk_parse_key(&(tlsDataParams->pkey), (unsigned char const *) pNetwork->tlsConnectParams.pDevicePrivateKeyLocation, (size_t) 0 , (unsigned char const *)"", 0);
  /* the key is converted to an RSA structure here :  pk_parse_key_pkcs1_der
     the info pointer are change in pk_wrap.c*/
  extern mbedtls_pk_info_t mbedtls_firewall_info;
  (tlsDataParams->pkey).pk_info = &mbedtls_firewall_info;
#else
  ret = mbedtls_pk_parse_key(&(tlsDataParams->pkey), (unsigned char const *) pNetwork->tlsConnectParams.pDevicePrivateKeyLocation, strlen(pNetwork->tlsConnectParams.pDevicePrivateKeyLocation) + 1, (unsigned char const *)"", 0);
#endif

  if(ret != 0)
  {
    msg_error(" failed\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\n\n", -ret);
    msg_debug(" path : %s ", pNetwork->tlsConnectParams.pDevicePrivateKeyLocation);
    return NETWORK_PK_PRIVATE_KEY_PARSE_ERROR;
  }
  msg_debug(" ok\n");
  char portBuffer[6];
  snprintf(portBuffer, 6, "%d", pNetwork->tlsConnectParams.DestinationPort);
  msg_debug("  . Connecting to %s/%s...", pNetwork->tlsConnectParams.pDestinationURL, portBuffer);

  if( (ret = net_sock_create(hnet, (net_sockhnd_t *)&tlsDataParams->server_fd.fd, NET_PROTO_TCP)) != NET_OK )
  {
    msg_error(" failed to create a TCP socket  ! net_sock_create %d\n", ret);
    return SSL_CONNECTION_ERROR;
  }

   if( (ret = net_sock_setopt(tlsDataParams->server_fd.fd, "sock_noblocking", NULL, 0)) != NET_OK )
   {
     msg_error(" failed to set the TCP socket noblocking ! net_sock_setopt %d\n", ret);
      return SSL_CONNECTION_ERROR;
   }
  
  if( (ret = net_sock_open(tlsDataParams->server_fd.fd, pNetwork->tlsConnectParams.pDestinationURL, pNetwork->tlsConnectParams.DestinationPort, 0) ) != NET_OK)
  {
    msg_error(" failed to connect to %s  ! net_sock_open returned %d\n", pNetwork->tlsConnectParams.pDestinationURL, ret);
    return SSL_CONNECTION_ERROR;
  }

  msg_debug("  . Setting up the SSL/TLS structure...");

  if((ret = mbedtls_ssl_config_defaults(&(tlsDataParams->conf), MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    msg_error(" failed\n  ! mbedtls_ssl_config_defaults returned -0x%x\n\n", -ret);
    return SSL_CONNECTION_ERROR;

  }

#ifndef MOSQUITTO
  mbedtls_ssl_conf_cert_profile( &(tlsDataParams->conf), &mbedtls_x509_crt_amazon_suite );
#endif
  mbedtls_ssl_conf_verify(&(tlsDataParams->conf), _iot_tls_verify_cert, NULL);
  if(pNetwork->tlsConnectParams.ServerVerificationFlag == true)
  {
    mbedtls_ssl_conf_authmode(&(tlsDataParams->conf), MBEDTLS_SSL_VERIFY_REQUIRED);
  }
  else
  {
    mbedtls_ssl_conf_authmode(&(tlsDataParams->conf), MBEDTLS_SSL_VERIFY_OPTIONAL);
  }
  mbedtls_ssl_conf_rng(&(tlsDataParams->conf), mbedtls_ctr_drbg_random, &(tlsDataParams->ctr_drbg));

  mbedtls_ssl_conf_ca_chain(&(tlsDataParams->conf), &(tlsDataParams->cacert), NULL);
  if((ret = mbedtls_ssl_conf_own_cert(&(tlsDataParams->conf), &(tlsDataParams->clicert), &(tlsDataParams->pkey))) !=
      0)
  {
    msg_error(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
    return SSL_CONNECTION_ERROR;
  }

  mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), pNetwork->tlsConnectParams.timeout_ms);

  if((ret = mbedtls_ssl_setup(&(tlsDataParams->ssl), &(tlsDataParams->conf))) != 0)
  {
    msg_error(" failed\n  ! mbedtls_ssl_setup returned -0x%x\n\n", -ret);
    return SSL_CONNECTION_ERROR;
  }
#ifdef MOSQUITTO
  /* If the server IP address cannot be resolved from a hostname, the expected server common name must be set manually. */ 
  char *common_name = "gnbiotsrv";
#else
  char *common_name = pNetwork->tlsConnectParams.pDestinationURL;
#endif
  if((ret = mbedtls_ssl_set_hostname(&(tlsDataParams->ssl), common_name )) != 0)
  {
    msg_error(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
    return SSL_CONNECTION_ERROR;
  }
  msg_debug("\n\nSSL state connect : %d ", tlsDataParams->ssl.state);
  mbedtls_ssl_set_bio(&(tlsDataParams->ssl), (void*) tlsDataParams->server_fd.fd, mbedtls_net_send, mbedtls_net_recv, NULL);
  msg_debug(" ok\n");

  msg_debug("\n\nSSL state connect : %d ", tlsDataParams->ssl.state);
  msg_debug("  . Performing the SSL/TLS handshake...");
  while((ret = mbedtls_ssl_handshake(&(tlsDataParams->ssl))) != 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      msg_error(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n", -ret);
      if(ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED)
      {
        msg_error("    Unable to verify the server's certificate. "
                  "Either it is invalid,\n"
                  "    or you didn't set ca_file or ca_path "
                  "to an appropriate value.\n"
                  "    Alternatively, you may want to use "
                  "auth_mode=optional for testing purposes.\n");
      }
      return SSL_CONNECTION_ERROR;
    }
  }

  msg_debug(" ok\n    [ Protocol is %s ]\n    [ Ciphersuite is %s ]\n", mbedtls_ssl_get_version(&(tlsDataParams->ssl)),
            mbedtls_ssl_get_ciphersuite(&(tlsDataParams->ssl)));
  if((ret = mbedtls_ssl_get_record_expansion(&(tlsDataParams->ssl))) >= 0)
  {
    msg_debug("    [ Record expansion is %d ]\n", ret);
  }
  else
  {
    msg_debug("    [ Record expansion is unknown (compression) ]\n");
  }

  msg_debug("  . Verifying peer X.509 certificate...");

  if(pNetwork->tlsConnectParams.ServerVerificationFlag == true)
  {
    if((tlsDataParams->flags = mbedtls_ssl_get_verify_result(&(tlsDataParams->ssl))) != 0)
    {
      char vrfy_buf[512];
      msg_error(" failed\n");
      mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", tlsDataParams->flags);
      msg_error("%s\n", vrfy_buf);
      ret = SSL_CONNECTION_ERROR;
    }
    else
    {
      msg_debug(" ok\n");
      ret = SUCCESS;
    }
  }
  else
  {
    msg_debug(" Server Verification skipped\n");
    ret = SUCCESS;
  }

#if 0
#ifdef msg_debug
  if (mbedtls_ssl_get_peer_cert(&(tlsDataParams->ssl)) != NULL)
  {
    msg_debug("  . Peer certificate information    ...\n");
    mbedtls_x509_crt_info((char *) buf, sizeof(buf) - 1, "      ", mbedtls_ssl_get_peer_cert(&(tlsDataParams->ssl)));
    msg_debug("%s\n", buf);
  }
#endif
#endif // 0
  mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), IOT_SSL_READ_TIMEOUT);

  return (IoT_Error_t) ret;
}

IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len)
{
  size_t written_so_far = 0;
  bool isErrorFlag = false;
  int frags = 0, ret = 0;
  TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);

  for(written_so_far = 0, frags = 0;
      written_so_far < len && !has_timer_expired(timer); written_so_far += ret, frags++)
  {
    while(!has_timer_expired(timer) &&
          (ret = mbedtls_ssl_write(&(tlsDataParams->ssl), pMsg + written_so_far, len - written_so_far)) <= 0)
    {
      if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
        msg_error(" failed\n  ! mbedtls_ssl_write returned -0x%x\n\n", -ret);
        /* All other negative return values indicate connection needs to be reset.
        * Will be caught in ping request so ignored here */
        isErrorFlag = true;
        break;
      }
    }
    if(isErrorFlag)
    {
      break;
    }
  }

  *written_len = written_so_far;

  if(isErrorFlag)
  {
    return NETWORK_SSL_WRITE_ERROR;
  }
  else if(has_timer_expired(timer) && written_so_far != len)
  {
    return NETWORK_SSL_WRITE_TIMEOUT_ERROR;
  }

  return SUCCESS;
}

IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len)
{
  size_t rxLen = 0;
  bool isErrorFlag = false;
  bool isCompleteFlag = false;
  uint32_t timerLeftVal = left_ms(timer);
  TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);
  int ret = 0;

  do
  {
    mbedtls_ssl_conf_read_timeout(&(tlsDataParams->conf), timerLeftVal);
    ret = mbedtls_ssl_read(&(tlsDataParams->ssl), pMsg, len);
    if(ret >= 0)   /* 0 is for EOF */
    {
      rxLen += ret;
    }
    else if(ret != MBEDTLS_ERR_SSL_WANT_READ)
    {
      isErrorFlag = true;
    }

    /* All other negative return values indicate connection needs to be reset.
     * Will be caught in ping request so ignored here */

    if(rxLen >= len)
    {
      isCompleteFlag = true;
    }
    timerLeftVal = left_ms(timer);
  }
  while(!isErrorFlag && !isCompleteFlag && timerLeftVal > 0);

  *read_len = rxLen;

  if(0 == rxLen && !isErrorFlag)
  {
    return NETWORK_SSL_NOTHING_TO_READ;
  }
  else if(has_timer_expired(timer) && !isCompleteFlag)
  {
    return NETWORK_SSL_READ_TIMEOUT_ERROR;
  }

  return SUCCESS;
}

IoT_Error_t iot_tls_disconnect(Network *pNetwork)
{
  mbedtls_ssl_context *ssl = &(pNetwork->tlsDataParams.ssl);
  int ret = 0;
  do
  {
    ret = mbedtls_ssl_close_notify(ssl);
  }
  while(ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  /* All other negative return values indicate connection needs to be reset.
   * No further action required since this is disconnect call */
  ret = net_sock_close(pNetwork->tlsDataParams.server_fd.fd);
  ret |= net_sock_destroy(pNetwork->tlsDataParams.server_fd.fd);
  if (ret != NET_OK)
  {
    msg_error("net_sock_close() or net_sock_destroy() failed.\n");
  }
  pNetwork->tlsDataParams.server_fd.fd = (net_sockhnd_t) -1;

  return SUCCESS;
}

IoT_Error_t iot_tls_destroy(Network *pNetwork)
{
  TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);

  mbedtls_x509_crt_free(&(tlsDataParams->clicert));
  mbedtls_x509_crt_free(&(tlsDataParams->cacert));
  mbedtls_pk_free(&(tlsDataParams->pkey));
  mbedtls_ssl_free(&(tlsDataParams->ssl));
  mbedtls_ssl_config_free(&(tlsDataParams->conf));
  mbedtls_ctr_drbg_free(&(tlsDataParams->ctr_drbg));
  mbedtls_entropy_free(&(tlsDataParams->entropy));
  return SUCCESS;
}

#ifdef __cplusplus
}
#endif
