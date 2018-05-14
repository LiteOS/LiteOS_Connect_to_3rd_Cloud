/**
  ******************************************************************************
  * @file    iot_flash_config.h
  * @author  MCD Application Team
  * @brief   Header for configuration
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

#ifndef __iot_flash_config_H
#define __iot_flash_config_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#define USER_CONF_C2C_SOAPC_MAX_LENGTH  16
#define USER_CONF_C2C_USERID_MAX_LENGTH 16
#define USER_CONF_C2C_PSW_MAX_LENGTH    16   

#define USER_CONF_WIFI_SSID_MAX_LENGTH  32
#define USER_CONF_WIFI_PSK_MAX_LENGTH   64

#define USER_CONF_DEVICE_NAME_LENGTH    300   /**< Must be large enough to hold a complete connection string */
#define USER_CONF_SERVER_NAME_LENGTH    128
#define USER_CONF_TLS_OBJECT_MAX_SIZE   2048
#define USER_CONF_MAGIC                 0x0123456789ABCDEFuLL
   
typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  char oper_ap_code[USER_CONF_C2C_SOAPC_MAX_LENGTH];  /**< C2C sim operator access point code */
  char username[USER_CONF_C2C_USERID_MAX_LENGTH];     /**< C2C username */
  char password[USER_CONF_C2C_PSW_MAX_LENGTH];        /**< C2C password */
} c2c_config_t;

typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  char ssid[USER_CONF_WIFI_SSID_MAX_LENGTH];          /**< Wifi network SSID. */
  char psk[USER_CONF_WIFI_PSK_MAX_LENGTH];            /**< Wifi network PSK. */
  uint8_t security_mode;                              /**< Wifi network security mode. See @ref wifi_network_security_t definition. */
} wifi_config_t;

/* Bluemix key words */
#define QUICK_START_REG_NAME "QuickStart"
#define SIMPLE_REG_NAME      "SimpleReg"
#define ORG_ID_KEY           "OrgId"
#define DEVICE_TYPE_KEY      "DeviceType"
#define DEVICE_ID_KEY        "DeviceId"
#define TOKEN_KEY            "Token"
#define IBM_IOT_URL          "internetofthings.ibmcloud.com"

typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  char device_name[USER_CONF_DEVICE_NAME_LENGTH];
  char server_name[USER_CONF_SERVER_NAME_LENGTH];
} iot_config_t;
 
/** Static user configuration data which must survive reboot and firmware update.
 * Do not change the field order, due to firewall constraint the tls_device_key size must be placed at a 64 bit boundary.
 * Its size sould also be multiple of 64 bits.
 *
 * Depending on the available board peripherals, the c2c_config and wifi_config fields may not be used.
 */
typedef struct {
  char tls_root_ca_cert[USER_CONF_TLS_OBJECT_MAX_SIZE * 3]; /* Allow room for 3 root CA certificates */
  char tls_device_cert[USER_CONF_TLS_OBJECT_MAX_SIZE];
  /* Above structure member must be aligned on 256 byte boundary
     to match firewall constraint , same for the size. */
  char tls_device_key[USER_CONF_TLS_OBJECT_MAX_SIZE];
#ifdef USE_C2C
  c2c_config_t c2c_config;
#endif
#ifdef USE_WIFI
  wifi_config_t wifi_config;
#endif
  iot_config_t iot_config;
  uint64_t ca_tls_magic;        /**< The USER_CONF_MAGIC magic word signals that the TLS root CA certificates strings 
                                    (tls_root_ca_cert) are present in Flash. */
  uint64_t device_tls_magic;    /**< The USER_CONF_MAGIC magic word signals that the TLS device certificate and key
                                    (tls_device_cert and tls_device_key) are present in Flash. */
} user_config_t;

int enterPemString(char * read_buffer, size_t max_len);
int getInputString(char *inputString, size_t len);

int checkC2cCredentials(const char ** const oper_ap_code, const char ** const username, const char ** const password);
int updateC2cCredentials(const char ** const oper_ap_code, const char ** const username, const char ** const password);
int checkWiFiCredentials(const char ** const ssid, const char ** const psk, uint8_t * const security_mode);
int updateWiFiCredentials(const char ** const ssid, const char ** const psk, uint8_t * const security_mode);

int updateTLSCredentials(void);
int checkTLSRootCA(void);
int checkTLSDeviceConfig(void);
int getTLSKeys(const char ** const ca_cert, const char ** const device_cert, const char ** const private_key);

int setIoTDeviceConfig(iot_config_t *config);
int getIoTDeviceConfig(const char ** const name);
int checkIoTDeviceConfig(void);

#ifdef AWS
int getServerAddress(const char ** const address);
#endif /* AWS */

int updateFirmwareVersion(void);

#ifdef __cplusplus
}
#endif
#endif /* __iot_flash_config_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

