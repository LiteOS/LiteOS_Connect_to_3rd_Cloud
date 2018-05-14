/******************************************************************************
* File       : usr_cfg.h
* Function   : Configuration of user.
* Description: Input Product ID, Device name, User Name, Password, Client ID
*              Publish topic and Subscribe topic.           
* Version    : V1.00
* Author     : Ian
* Date       : 13th May 2018
* History    :  No.  When           Who           What
*               1    13/May/2018    Ian           Create
******************************************************************************/
#ifndef _USR_CFG_H_
#define _USR_CFG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define LOS_IOT_PRODUCT_ID          "DXJQTLK47X"
#define LOS_IOT_DEV_NAME            "Sensor1"
#define LOS_IOT_USR_NAME            "DXJQTLK47XSensor1;21010406;12365;1526231296"
#define LOS_IOT_PASSWORD            "d73fccb85d34dcf6debb40cbef11a92822c9de62;hmacsha1"

#define LOS_IOT_PUB_DATA            "LiteOS for IoT!!"
  
#define LOS_IOT_CLIENT_ID           "DXJQTLK47XSensor1"
#define LOS_IOT_PUB_TOPIC           "DXJQTLK47X/Sensor1/event"
#define LOS_IOT_SUB_TOPIC           "DXJQTLK47X/Sensor1/control"
  
#define LOS_IOT_LOCAL_IP1            192
#define LOS_IOT_LOCAL_IP2            168
#define LOS_IOT_LOCAL_IP3            31
#define LOS_IOT_LOCAL_IP4            195
  
#define LOS_IOT_GW1                  192
#define LOS_IOT_GW2                  168
#define LOS_IOT_GW3                  31
#define LOS_IOT_GW4                  1

#ifdef __cplusplus
}
#endif

#endif /* _USR_CFG_H_ */

/* End of file */
