/**
  ******************************************************************************
  * @file    subscribe_publish_sensor_values.c
  * @author  MCD Application Team
  * @brief   Control of the measurement sampling and MQTT reporting loop.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "iot_flash_config.h"
#include "sensors_data.h"
#include "msg.h"

const char *GlobalCaCert = "MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\
yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\
ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\
U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\
ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\
aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\
MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\
ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\
biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\
U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\
aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\
nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\
t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\
SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\
BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\
rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\
NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\
BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\
BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\
aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\
MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\
p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\
5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\
WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\
4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\
hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq";
  const char *GlobalClientCert = "MIIDWTCCAkGgAwIBAgIUCYy7CjQmeekJRl9air+SyTOTRQwwDQYJKoZIhvcNAQEL\
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE4MDUxMjAxMjUw\
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKu5PODE7f73hqMQj+tf\
dF29QCs1WzRuotOB5IJPjJ5auecJbVTGv7DfSgNJg2tjgxzue+Jdm7+DHT+IZjt2\
tfqSFLtWoVKExT2oXSlxEdhdBVBDs3MH9PqayAFN5RblqCYS+nBUxK2C0oMg3xu0\
YWuEhA1gsA2LeymTjePTcDiyfP/VcSyHihQ8MJiOTWkasryhgVhC/DcXrTY7/bgu\
+QeeHxFk5UgkStr5kBK5KN+7hiHJFI5+QrrLWRPISRVlYAQMzm/CzfOtlAjSx21r\
w9mpytBUdAvO2X9Rz0s9HGU4+bInF0iNbla6nFCb//gKTjqZj2y+ML/r2isddMaF\
pYsCAwEAAaNgMF4wHwYDVR0jBBgwFoAUEDHMb0MAH/7/b5vy5WDjiD06d/IwHQYD\
VR0OBBYEFM9tHISVi6BTgV5y0O1Oeiht0OlDMAwGA1UdEwEB/wQCMAAwDgYDVR0P\
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCdOXhND/zra32TFh+cbQOia5WX\
WgWAkr24ng4JQ9s4PeaRejD2u49xlEK39KT4Yd7/VvCXarvD7iSai0LD1OzzLQcb\
k4AiNbYA7RkdDNlE/0mDVCxqe8zKjnNbmdwvYxpVLKdwdNB2cEEyMzcNT4IEy/y3\
wulxpEHGzIvWdQ1hxDT76mCJx/RjHRUTbZtJT3KHLorA3c7KQvnPQbJkabItErkp\
IRaLTieO3QYxxgnDSivegOCKHJ+42pUEZ2DT1V13vKusArGWgJkCfz4m6qNiuBO7\
6KxzZsT9oNe3W2JAGlbuUUGnES5psrzMceXmvOdM1tDtxA92/alg3qHIbsGw";
  const char *GlobalClientPrivateKey = "MIIEpQIBAAKCAQEAq7k84MTt/veGoxCP6190Xb1AKzVbNG6i04Hkgk+Mnlq55wlt\
VMa/sN9KA0mDa2ODHO574l2bv4MdP4hmO3a1+pIUu1ahUoTFPahdKXER2F0FUEOz\
cwf0+prIAU3lFuWoJhL6cFTErYLSgyDfG7Rha4SEDWCwDYt7KZON49NwOLJ8/9Vx\
LIeKFDwwmI5NaRqyvKGBWEL8NxetNjv9uC75B54fEWTlSCRK2vmQErko37uGIckU\
jn5CustZE8hJFWVgBAzOb8LN862UCNLHbWvD2anK0FR0C87Zf1HPSz0cZTj5sicX\
SI1uVrqcUJv/+ApOOpmPbL4wv+vaKx10xoWliwIDAQABAoIBAEaMN0JwJQZWUm2z\
ZCzibkAEe9REe0+zFBmoh2Qxoid8RltwOAZYEHBti06GaQM5QqgkIakBo/atutpb\
AwTy5Bgo43ODR4tLGG0YOg3IcfhdXs12wsZDQpWh+QDwK2mYt9fMfC6h5JsT38AX\
RRFwrYUqT8XfLvShlNujzusF9liT7xIUdikCHgKN9h24hbe76hLu+I8xKDa6U/j0\
VknjMSuOZUQG1L/ARgSmEAQRzcOztD0FdcJfv8wmZHBujsvb1YMU4P8xTgUeozwT\
QsTXIuEomnoNaKvTQ2jPYyj30KmMJFzTLyK9faSIeAOZICB8e3781F+oa7MfZSIs\
PISig8ECgYEA1RY2nPmZkh/7OKO34/IB1ip/hiC3AzR8UnOC/ybxdZFvZDIVpcJQ\
bSnXfD8scvB4g1DHslY+SlplfJ7vwaxSwzmbTxZysa96hELK0zcq0iZrgqjALi9p\
6yuhKIJFD6AGLoOtRnGRG76pHlyBUJ8LdVUAGF0DH7G2N6gVpme+CZMCgYEAzk6G\
wHIjQxnUpO70k9Te9U2gGn9f76e4ze5rahDANCy+twvE6jCzN4ReFlD3Ek1q3sEX\
apar4x34s24V4iwUsPVl285uwONceeXXSGDBpoGTvtSN5T7TAXTxO1amy8JM3buB\
pqSWAqEjXMAGuGJYh3kqEpLcY75ZTqwX1JlvjykCgYEAkfVcRVwnWFN1xo9GV+oe\
Xe/QWDSyh4x9GBEtis7HGGwda/luoJMgehXuF5pCPR0Av5omn765fj67Q4iu2+/I\
5RbhjmPm/lUfy9bgjZs9RqOwgthKg5hQqhjBlDjfS1umwFfL6fPCnMmVmElu/qVi\
uOUXYpjbmesl0Kve4JQxNOcCgYEAwGvRl1FdvkorYaJWkEOtqNI8EDrZeifPYJON\
sCHrgz5wqz3Y7i16Gr/lGZ9usq1ujlE+y/W4YF8mBgUZeBhxPBPGa1uBRXtKV4+e\
Wi2UsP6OlSCfMECGggdWsGfDrqyNEQrAhHNiTxWIbAvWuMdRlxnVufksggOaPJya\
pXpu6PECgYEAiaEKcfqOyhxNC10F1LrAz/o+IM933rMHwMKok5d1AxgmMqwPG1pM\
GaDBEapPc+Ox0GGXjhjwCYl26DFJpQnE1Wk/FnORDZg5JsQWqbbhAoYxv/c9bXGz\
bCNCVvIO/S0ObFdQ3xvqtu1oBOCQEfaGCzLJJrRuk2MmSK0VYqPA8v8=";


void MQTTcallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData);
int subscribe_publish_sensor_values(void);

/* Private defines ------------------------------------------------------------*/
#define MQTT_CONNECT_MAX_ATTEMPT_COUNT 3
#define TIMER_COUNT_FOR_SENSOR_PUBLISH 10

#define aws_json_pre        "{\"state\":{\"reported\":"
#define aws_json_desired    "{\"state\":{\"desired\":"
#define aws_json_post       "}}"

/* Private variables ---------------------------------------------------------*/
static char ledstate[] = { "Off" };
static char cPTopicName[MAX_SHADOW_TOPIC_LENGTH_BYTES] = "";
static char cSTopicName[MAX_SHADOW_TOPIC_LENGTH_BYTES] = "";
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
* @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
*/
static uint32_t publishCount = 60;

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
int cloud_device_enter_credentials(void)
{
  int ret = 0;
  iot_config_t iot_config;

  memset(&iot_config, 0, sizeof(iot_config_t));
    
  printf("\nEnter server address: (example: xxx.iot.region.amazonaws.com) \n");
  getInputString(iot_config.server_name, USER_CONF_SERVER_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.server_name);
  
  printf("\nEnter device name: (example: mything1) \n");
  getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

  if(setIoTDeviceConfig(&iot_config) != 0)
  {
    ret = -1;
    msg_error("Failed programming the IoT device configuration to Flash.\n");
  }
  
  return ret;
}

bool app_needs_device_keypair()
{
  return true;
}


/**
* @brief MQTT disconnect callback hander
*
* @param pClient: pointer to the AWS client structure
* @param data: 
* @return no return
*/
static void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data)
{
  msg_warning("MQTT Disconnect\n");
  IoT_Error_t rc = FAILURE;
  
  if(NULL == data)
  {
    return;
  }

  AWS_IoT_Client *client = (AWS_IoT_Client *)data;

  if(aws_iot_is_autoreconnect_enabled(client))
  {
    msg_info("Auto Reconnect is enabled, Reconnecting attempt will start now\n");
  }
  else
  {
    msg_warning("Auto Reconnect not enabled. Starting manual reconnect...\n");
    rc = aws_iot_mqtt_attempt_reconnect(client);

    if(NETWORK_RECONNECTED == rc)
    {
      msg_warning("Manual Reconnect Successful\n");
    }
    else
    {
      msg_warning("Manual Reconnect Failed - %d\n", rc);
    }
  }
}

/* Exported functions --------------------------------------------------------*/

/**
* @brief MQTT subscriber callback hander
*
* called when data is received from AWS IoT Thing (message broker)
* @param MQTTCallbackParams type parameter
* @return no return
*/
void MQTTcallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData)
{
  const char msg_on[]  = "{\"state\":{\"reported\":{\"LED_value\":\"On\"}}}";
  const char msg_off[] = "{\"state\":{\"reported\":{\"LED_value\":\"Off\"}}}";
  const char *msg = NULL;
  IoT_Publish_Message_Params paramsQOS1 = {QOS1, 0, 0, 0, NULL,0};
  
  msg_info("\nMQTT subscribe callback......\n");
  msg_info("%.*s\n", (int)params->payloadLen, (char *)params->payload);
  
  /* If a new desired LED state is received, change the LED state. */
  if (strstr((char *) params->payload, "\"desired\":{\"LED_value\":\"On\"}") != NULL)
  {
//    Led_SetState(true);
    strcpy(ledstate, "On");    
    msg_info("LED On!\n");
    msg = msg_on;
  }
  else if (strstr((char *) params->payload, "\"desired\":{\"LED_value\":\"Off\"}") != NULL)
  {
//    Led_SetState(false);
    strcpy(ledstate, "Off");
    msg_info("LED Off!\n");
    msg = msg_off;
  }
  
  /* Report the new LED state to the MQTT broker. */
  if (msg != NULL)
  { 
    paramsQOS1.payload = (void *) msg;
    paramsQOS1.payloadLen = strlen(msg) + 1;
    IoT_Error_t rc = aws_iot_mqtt_publish(pClient, cPTopicName, strlen(cPTopicName), &paramsQOS1);

    if (rc == AWS_SUCCESS)
    {
      msg_info("\nPublished the new LED status to topic %s:", cPTopicName);
      msg_info("%s\n", msg);
    }
  }
}

/**
* @brief main entry function to AWS IoT code
*
* @param no parameter
* @return AWS_SUCCESS: 0 
          FAILURE: -1
*/
int subscribe_publish_sensor_values(void)
{
  bool infinitePublishFlag = true;
  //const char *serverAddress = NULL;
	const char *serverAddress = "a1olq1gp3frg18.iot.eu-west-1.amazonaws.com";
  //const char *pCaCert;
//	const char *pCaCert;
//  const char *pClientCert;
//  const char *pClientPrivateKey;
	const char *pCaCert = GlobalCaCert;
  const char *pClientCert = GlobalClientCert;
  const char *pClientPrivateKey = GlobalClientPrivateKey;
  //const char *pDeviceName;
	const char *pDeviceName = "garbagecan01";
  char cPayload[AWS_IOT_MQTT_TX_BUF_LEN];
  //char const * deviceName;
	char const * deviceName = "garbagecan01";
  int i = 0;
  int connectCounter;
  IoT_Error_t rc = FAILURE;
#ifdef SENSOR
  int timeCounter = 0;
#endif
  uint8_t bp_pushed;

  AWS_IoT_Client client;
  memset(&client, 0, sizeof(AWS_IoT_Client));
  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  //getIoTDeviceConfig(&deviceName);

  if (strlen(deviceName) >= MAX_SIZE_OF_THING_NAME)
  {
    msg_error("The length of the device name stored in the iot user configuration is larger than the AWS client MAX_SIZE_OF_THING_NAME.\n");
    return -1;
  }
  
  snprintf(cPTopicName, sizeof(cPTopicName), AWS_DEVICE_SHADOW_PRE "%s" AWS_DEVICE_SHADOW_UPDATE_TOPIC, deviceName);
  snprintf(cSTopicName, sizeof(cSTopicName), AWS_DEVICE_SHADOW_PRE "%s" AWS_DEVICE_SHADOW_UPDATE_ACCEPTED_TOPIC, deviceName);
  
  /*
  IoT_Publish_Message_Params paramsQOS0;
  IoT_Publish_Message_Params paramsQOS1;
  */

  msg_info("AWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

  //getServerAddress(&serverAddress);
  //getTLSKeys(&pCaCert, &pClientCert, &pClientPrivateKey);
  mqttInitParams.enableAutoReconnect = false; /* We enable this later below */
  mqttInitParams.pHostURL = (char *) serverAddress;
  mqttInitParams.port = AWS_IOT_MQTT_PORT;
  mqttInitParams.pRootCALocation = (char *) pCaCert;
  mqttInitParams.pDeviceCertLocation = (char *) pClientCert;
  mqttInitParams.pDevicePrivateKeyLocation = (char *) pClientPrivateKey;
  mqttInitParams.mqttCommandTimeout_ms = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms = 5000;
  mqttInitParams.isSSLHostnameVerify = true;
  mqttInitParams.disconnectHandler = disconnectCallbackHandler;
  mqttInitParams.disconnectHandlerData = NULL;

  rc = aws_iot_mqtt_init(&client, &mqttInitParams);

  if(AWS_SUCCESS != rc)
  {
    msg_error("aws_iot_mqtt_init returned error : %d\n", rc);
    return -1;
  }

  //getIoTDeviceConfig(&pDeviceName);
  connectParams.keepAliveIntervalInSec = 30;
  connectParams.isCleanSession = true;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.pClientID = (char *) pDeviceName;
  connectParams.clientIDLen = (uint16_t) strlen(pDeviceName);
  connectParams.isWillMsgPresent = false;


  connectCounter = 0;
  
  do 
  {
    connectCounter++;
    printf("MQTT connection in progress:   Attempt %d/%d ...\n",connectCounter,MQTT_CONNECT_MAX_ATTEMPT_COUNT);
    rc = aws_iot_mqtt_connect(&client, &connectParams);
  } while((rc != AWS_SUCCESS) && (connectCounter < MQTT_CONNECT_MAX_ATTEMPT_COUNT));  

  if(AWS_SUCCESS != rc) 
  {
    msg_error("Error(%d) connecting to %s:%d\n", rc, mqttInitParams.pHostURL, mqttInitParams.port);
    return -1;
  }
  else
  {
    printf("Connected to %s:%d\n", mqttInitParams.pHostURL, mqttInitParams.port);
  }

  /*
  * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
  *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
  *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
  */
  rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
  
  if(AWS_SUCCESS != rc)
  {
    msg_error("Unable to set Auto Reconnect to true - %d\n", rc);

    if (aws_iot_mqtt_is_client_connected(&client)) 
    {
      aws_iot_mqtt_disconnect(&client);
    }

    return -1;
  }
  
  rc = aws_iot_mqtt_subscribe(&client, cSTopicName, strlen(cSTopicName), QOS0, MQTTcallbackHandler, NULL);

  if(AWS_SUCCESS != rc)
  {
    msg_error("Error subscribing : %d\n", rc);
    return -1;
  } 
  else
  {
    msg_info("Subscribed to topic %s\n", cSTopicName);
  }

  sprintf(cPayload, "%s : %d ", "hello from STM", i);

  IoT_Publish_Message_Params paramsQOS1 = {QOS1, 0, 0, 0, NULL,0};
  paramsQOS1.payload = (void *) cPayload;

  if(publishCount != 0)
  {
    infinitePublishFlag = false;
  }
  
  
  printf("Press the User button (Blue) to publish the LED desired value on the %s topic\n", cPTopicName);
  
#ifdef SENSOR
  timeCounter = TIMER_COUNT_FOR_SENSOR_PUBLISH;
#endif
   
  while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || AWS_SUCCESS == rc) && (publishCount > 0 || infinitePublishFlag))
  {
    /* Max time the yield function will wait for read messages */
    rc = aws_iot_mqtt_yield(&client, 10);

    if(NETWORK_ATTEMPTING_RECONNECT == rc)
    {
      /* Delay to let the client reconnect */
      //HAL_Delay(1000); 
      msg_info("Attempting to reconnect\n");
      /* If the client is attempting to reconnect we will skip the rest of the loop */
      continue; 
    }
    if(NETWORK_RECONNECTED == rc)
    {
      msg_info("Reconnected.\n");
    }

 //   bp_pushed = Button_WaitForMultiPush(1000);
    
    /* exit loop on long push  */
 /*   if (bp_pushed == BP_MULTIPLE_PUSH ) 
    {
      msg_info("\nPushed button perceived as a *double push*. Terminates the application.\n");
      infinitePublishFlag = false;
      publishCount = 0;
      break;
    }
    
    if (bp_pushed == BP_SINGLE_PUSH)
    {
      if(strstr(ledstate, "Off")!= NULL)
      {
        strcpy(ledstate, "On");
      }
      else
      {
        strcpy(ledstate, "Off");
      }
      
      printf("Sending the desired LED state to AWS.\n");
    */  
      /* create desired message */
 /*     memset(cPayload, 0, sizeof(cPayload));
      strcat(cPayload, aws_json_desired);
      strcat(cPayload, "{\"LED_value\":\"");
      strcat(cPayload, ledstate);
      strcat(cPayload, "\"}");
      strcat(cPayload, aws_json_post);
      
      paramsQOS1.payloadLen = strlen(cPayload) + 1;

      do 
      {
        rc = aws_iot_mqtt_publish(&client, cPTopicName, strlen(cPTopicName), &paramsQOS1);

        if (rc == AWS_SUCCESS)
        {
          printf("\nPublished to topic %s:", cPTopicName);
          printf("%s\n", cPayload);
        }

        if (publishCount > 0)
        {
          publishCount--;
        }
      } while(MQTT_REQUEST_TIMEOUT_ERROR == rc && (publishCount > 0 || infinitePublishFlag));      
    }
*/    
#ifdef  SENSOR
    timeCounter ++;
    if (timeCounter >= TIMER_COUNT_FOR_SENSOR_PUBLISH)  
    {
      timeCounter = 0;
            
      PrepareSensorsData(cPayload, sizeof(cPayload), NULL);           
            
      paramsQOS1.payloadLen = strlen(cPayload) + 1;
            
      do
      {
        rc = aws_iot_mqtt_publish(&client, cPTopicName, strlen(cPTopicName), &paramsQOS1);

        if (rc == AWS_SUCCESS)
        {
          printf("\nPublished to topic %s:\n", cPTopicName);
          printf("%s\n", cPayload);
        }

        if (publishCount > 0)
        {
          publishCount--;
        }
      } while((MQTT_REQUEST_TIMEOUT_ERROR == rc) && (publishCount > 0 || infinitePublishFlag));
    }
#endif


    
  } /* End of while */
  
  /* Wait for all the messages to be received */
  aws_iot_mqtt_yield(&client, 10);

  rc = aws_iot_mqtt_disconnect(&client);


  return rc;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
