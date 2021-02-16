/**
 ******************************************************************************
  * File Name          : LWIP.c
  * Description        : This file provides initialization code for LWIP
  *                      middleWare.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "diag.h"
/* Moreno */
#if 1
    #include "counter.h"
    #include "tiny-httpd.h"
#endif

/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

#define MAX_DHCP_TRIES 250

enum DCHP_LINK_STATUS{NOT_INITIALIZED, LINK_UP_WAITING_DHCP, LINK_UP_IP_ACQUIRED, LINK_UP_DHCP_ERROR, LINK_DOWN, LINK_DOWN_INIT};
static enum DCHP_LINK_STATUS dhcp_link_status = NOT_INITIALIZED;

/* Semaphore to signal Ethernet Link state update */
osSemaphoreId Netif_LinkSemaphore = NULL;
/* Ethernet link thread Argument */
struct link_str link_arg;

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void)
{
#if 1
  /* Initialize the LwIP stack with RTOS */
  tcpip_init( NULL, NULL );
#else
    /* Michrochip example: it does not release the semaphore, but good to know how it works
     with LwIP semaphore */
    sys_sem_t sem;
    err_t err_sem;
    err_sem = sys_sem_new(&sem, 0); /* Create a new semaphore. */
    //tcpip_init(tcpip_init_done, &sem);
    tcpip_init(NULL, &sem);
    sys_sem_wait(&sem); /* Block until the lwIP stack is initialized. */
    sys_sem_free(&sem); /* Free the semaphore. */
#endif
  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

  /* add the network interface (IPv4/IPv6) with RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
    err_t err = dhcp_start(&gnetif);
    if (err) {
    	dhcp_link_status = LINK_UP_DHCP_ERROR;
    }
    else{
    	dhcp_link_status = LINK_UP_WAITING_DHCP;
    }
    RAW_DIAG("Link up");
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
    dhcp_link_status = LINK_DOWN_INIT;
	RAW_DIAG("Link down");
  }

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernetif_update_config);

  /* create a binary semaphore used for informing ethernetif of frame reception */
  osSemaphoreDef(Netif_SEM);
  Netif_LinkSemaphore = osSemaphoreCreate(osSemaphore(Netif_SEM) , 1 );

  link_arg.netif = &gnetif;
  link_arg.semaphore = Netif_LinkSemaphore;
  /* Create the Ethernet link handler thread */
  /* USER CODE BEGIN OS_THREAD_DEF_CREATE_CMSIS_RTOS_V1 */
    /* Moreno: Begin */
    /* As in the Microchip example, the webserver (any any network related busyness) is
     controlled by LwIP by
     sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio);
     name: human-readable name for the thread (used for debugging purposes)
     thread: thread-function
     arg: parameter passed to 'thread'
     stacksize: stack size in bytes for the new thread (may be ignored by ports)
     prio: priority of the new thread (may be ignored by ports)
     */
    sys_thread_new("Tiny-HTTPD", tinyd, NULL, 1024, osPriorityNormal);
    /* Moreno: End */
  osThreadDef(LinkThr, ethernetif_set_link, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
  osThreadCreate (osThread(LinkThr), &link_arg);
/* USER CODE END OS_THREAD_DEF_CREATE_CMSIS_RTOS_V1 */
}

/**
  * @brief  This function sets the netif link status.
  * @param  netif: the network interface
  * @retval None
  */
void ethernetif_set_link(void const *argument)
{
  uint32_t regvalue = 0;
  struct link_str *link_arg = (struct link_str *)argument;

  for(;;)
  {
#if 1
      begin_count(&counter_ethernetif_set_link);
#endif
	  switch(dhcp_link_status){
	  case NOT_INITIALIZED:
		  break;
	  case LINK_UP_WAITING_DHCP:
		  regvalue = ethernetif_phy_link_status_bit();
		  if(netif_is_link_up(link_arg->netif) && (!regvalue)){
			  dhcp_stop(&gnetif);
			  netif_set_link_down(link_arg->netif);
			  dhcp_link_status = LINK_DOWN;
			  RAW_DIAG("Link down");
			  break;
		  }
		  if (dhcp_supplied_address(&gnetif)) {
			  uint8_t iptxt[20];
			  sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&gnetif.ip_addr));
			  RAW_DIAG("IP address assigned by a DHCP server: %s", iptxt);
			  dhcp_link_status = LINK_UP_IP_ACQUIRED;
			  break;
		  }
		  struct dhcp *dhcp = (struct dhcp *)netif_get_client_data(&gnetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
		  if (dhcp->tries > MAX_DHCP_TRIES){
			  dhcp_stop(&gnetif);
			  dhcp_link_status = LINK_UP_DHCP_ERROR;
			  break;
		  }
		  break;
	  case LINK_UP_IP_ACQUIRED:
		  regvalue = ethernetif_phy_link_status_bit();
		  if(netif_is_link_up(link_arg->netif) && (!regvalue)){
			  dhcp_stop(&gnetif);
			  netif_set_link_down(link_arg->netif);
			  dhcp_link_status = LINK_DOWN;
			  RAW_DIAG("Link down");
		  }
		  break;
	  case LINK_UP_DHCP_ERROR:
		  regvalue = ethernetif_phy_link_status_bit();
		  if(netif_is_link_up(link_arg->netif) && (!regvalue)){
			  dhcp_stop(&gnetif);
			  netif_set_link_down(link_arg->netif);
			  dhcp_link_status = LINK_DOWN;
			  RAW_DIAG("Link down");
			  break;
		  }
		  dhcp_stop(&gnetif);

		  /* Static address used */
		  IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
		  IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
		  IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
		  netif_set_addr(&gnetif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));

		  uint8_t iptxt[20];
		  sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&gnetif.ip_addr));
		  RAW_DIAG("DHCP error and/or timeout !!");
		  RAW_DIAG("Static IP address: %s\n", iptxt);
		  dhcp_link_status = LINK_UP_IP_ACQUIRED;
		  break;
	  case LINK_DOWN:
		  regvalue = ethernetif_phy_link_status_bit();
		  if(!netif_is_link_up(link_arg->netif) && (regvalue)){
			  netif_set_link_up(link_arg->netif);
			  err_t err = dhcp_start(&gnetif);
			  if (err) {
				  dhcp_link_status = LINK_UP_DHCP_ERROR;
			  }
			  else{
				  dhcp_link_status = LINK_UP_WAITING_DHCP;
			  }
			  RAW_DIAG("Link up");
			  break;
		  }
		  break;
	  case LINK_DOWN_INIT:
		  regvalue = ethernetif_phy_link_status_bit();
		  if(regvalue){
			  netif_set_up(&gnetif);
			  netif_set_link_up(link_arg->netif);
			  err_t err = dhcp_start(&gnetif);
			  if (err) {
				  dhcp_link_status = LINK_UP_DHCP_ERROR;
			  }
			  else{
				  dhcp_link_status = LINK_UP_WAITING_DHCP;
			  }
			  RAW_DIAG("Link up");
			  break;
		  }
		  break;
	  }
#if 1
      end_count(&counter_ethernetif_set_link);
#endif
    osDelay(1000);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
