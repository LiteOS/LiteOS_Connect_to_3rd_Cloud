/*********************************************Copyright (c)***********************************************
** Wuhan CCEM
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               
** Latest modified Date:    
** Latest Version:          V1.0
** Descriptions:            Contains header files and defined the structures
**
**--------------------------------------------------------------------------------------------------------
** Created by:              shhuang
** Created date:            2018-05-11
** Version:                 V1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/
#ifndef __IOTHUB_MQTT_CLIENT_H
#define __IOTHUB_MQTT_CLIENT_H
#define __INTELLIGENT_FIRE_BAIDUCLOUD_H
#define __TRANSPORT_H
#ifdef      __cplusplus
extern  "C" {
#endif

/********************************************************************************************************
** Includes
*********************************************************************************************************/

extern unsigned int g_usGetNetStatusSemID;
extern	unsigned int g_usGetNetDataSemID;

/*********************************************************************************************************
** Function declaration
*********************************************************************************************************/ 
void connect_mqtt_server(void);
void disconnect_mqtt_server(void);
void mqtt_client_pubtopic(void);



#ifdef __cplusplus
}
#endif                                                                  

#endif                                                                  

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/	






















































































