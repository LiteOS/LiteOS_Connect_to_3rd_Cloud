/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Network connection configuration
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "lwipopts/netconf.h"
#include <stdio.h>
#include "LAN8742A.h"
#include "tcpip.h"
#include "los_base.h"

/* Private typedef -----------------------------------------------------------*/
#define MAX_DHCP_TRIES        4

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif gnetif;
uint32_t IPaddress = 0;

#ifdef USE_DHCP
__IO uint8_t DHCP_state;
#endif
extern __IO uint32_t  EthStatus;
extern __IO INT32  DeviceState;

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Initializes the lwIP stack
* @param  None
* @retval None
*/
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	
  /* Create tcp_ip stack thread */
  tcpip_init( NULL, NULL );	
  
#ifdef USE_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else
  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif  

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
  struct ip_addr *netmask, struct ip_addr *gw,
  void *state, err_t (* init)(struct netif *netif),
  err_t (* input)(struct pbuf *p, struct netif *netif))

  Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
	
  /*  Registers the default network interface.*/
  netif_set_default(&gnetif);

  if (EthStatus == (ETH_INIT_FLAG | ETH_LINK_FLAG))
  { 
    /* Set Ethernet link flag */
    gnetif.flags |= NETIF_FLAG_LINK_UP;

    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);			
		
#ifdef USE_DHCP
    DHCP_state = DHCP_START;
#else

	  DeviceState = 1;
#ifdef SERIAL_DEBUG
		printf("\n  Static IP address   \r\n");
		printf("IP: %d.%d.%d.%d\r\n",IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
		printf("NETMASK: %d.%d.%d.%d\r\n",NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
		printf("Gateway: %d.%d.%d.%d\r\n",GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* SERIAL_DEBUG */
#endif /* USE_DHCP */
  }
  else
  {
    /*  When the netif link is down this function must be called.*/
    netif_set_down(&gnetif);
#ifdef USE_DHCP
    DHCP_state = DHCP_LINK_DOWN;
#endif /* USE_DHCP */
#ifdef SERIAL_DEBUG
		printf("\n  Network Cable is  \r\n");
		printf("    not connected   \r\n");
#endif /* SERIAL_DEBUG */
  }

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ETH_link_callback);
	
}


#ifdef USE_DHCP
/**
* @brief  LwIP_DHCP_Process_Handle
* @param  None
* @retval None
*/
void LwIP_DHCP_task(void * pvParameters)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
	
	while(1)
	{
		
		//printf("DHCP_state=%d\n",DHCP_state);
	
		switch (DHCP_state)
		{
		case DHCP_START:
			{
				DHCP_state = DHCP_WAIT_ADDRESS;
				dhcp_start(&gnetif);
				/* IP address should be set to 0 
					 every time we want to assign a new DHCP address */
				IPaddress = 0;
#ifdef SERIAL_DEBUG
				printf("\n     Looking for    \r\n");
				printf("     DHCP server    \r\n");
				printf("     please wait... \r\n");
#endif /* SERIAL_DEBUG */
			}
			break;

		case DHCP_WAIT_ADDRESS:
			{
				/* Read the new IP address */
				IPaddress = gnetif.ip_addr.addr;

				if (IPaddress!=0) 
				{
					DHCP_state = DHCP_ADDRESS_ASSIGNED;	

					/* Stop DHCP */
					dhcp_stop(&gnetif);					
					
					//IP获取成功
					DeviceState = 1;

#ifdef SERIAL_DEBUG
					printf("\n  IP address assigned \r\n");
					printf("    by a DHCP server   \r\n");
					printf("IP: %d.%d.%d.%d\r\n",(uint8_t)(IPaddress),(uint8_t)(IPaddress >> 8), \
																 (uint8_t)(IPaddress >> 16),(uint8_t)(IPaddress >> 24));
					printf("NETMASK: %d.%d.%d.%d\r\n",NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
					printf("Gateway: %d.%d.%d.%d\r\n",GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
					
					
#endif /* SERIAL_DEBUG */
				}
				else
				{
					/* DHCP timeout */
					if (gnetif.dhcp->tries > MAX_DHCP_TRIES)
					{
						DHCP_state = DHCP_TIMEOUT;

						/* Stop DHCP */
						dhcp_stop(&gnetif);

						/* Static address used */
						IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
						IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
						IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
						netif_set_addr(&gnetif, &ipaddr , &netmask, &gw);

#ifdef SERIAL_DEBUG
						printf("\n    DHCP timeout    \r\n");
						printf("  Static IP address   \r\n");
						printf("IP: %d.%d.%d.%d\r\n",IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
						printf("NETMASK: %d.%d.%d.%d\r\n",NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
						printf("Gateway: %d.%d.%d.%d\r\n",GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* SERIAL_DEBUG */
					}
				}
			}
			break;
		default: break;
		}
		
		LOS_Msleep(250);
	}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
