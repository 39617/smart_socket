#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"

#if PLATFORM_HAS_BUTTON
#include "dev/button-sensor.h"
#endif

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

static void http_init_engine(void) {}
static void http_set_service_callback(service_callback_t callback) {}


// TODO...
#include "er-coap.h"
#include "er-coap-engine.h"
struct rest_implementation http_rest_implementation = {0};

/*
 * Resources to be used.
 */
extern resource_t
  res_switch;



PROCESS(smart_socket, "Smart-Socket");
AUTOSTART_PROCESSES(&smart_socket);


/**
 * Sets the IP configuration for the RF interface.
 */
static void init_rf_if_addr(void) {
	if(!UIP_CONF_IPV6_RPL) {
	  uip_ipaddr_t ipaddr;
	  int i;

	  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, IPV6_CONF_ADDR_8);
	  //uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	  uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
	  printf("* Radio: Tentative site IPv6 address ");
	  for(i = 0; i < 7; ++i) {
		  printf("%02x%02x:",
			   ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
	  }
	  printf("%02x%02x\n",
			 ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
	}

	printf("* Radio: Tentative link-local IPv6 address ");
	{
	uip_ds6_addr_t *lladdr;
	int i;
	lladdr = uip_ds6_get_link_local(-1);
	for(i = 0; i < 7; ++i) {
		printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
			 lladdr->ipaddr.u8[i * 2 + 1]);
	}
	printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
	}
}



PROCESS_THREAD(smart_socket, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  PRINTF("* Starting Smart Socket process\n");

  init_rf_if_addr();

#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  // TODO: ...
  http_rest_implementation.init = http_init_engine;
  http_rest_implementation.set_service_callback = http_set_service_callback;
  rest_init_engine();

  /* Activate resources */
  rest_activate_resource(&res_switch, "switch");

  while(1) {
    PROCESS_WAIT_EVENT();
#if PLATFORM_HAS_BUTTON
    if(ev == sensors_event && data == &button_sensor) {
      PRINTF("*******BUTTON*******\n");

      // TODO: Ver isto do res_event.trigger - serve para os updates periodicos dos consumos
      /* Call the event_handler for this application-specific event. */
      //////////////res_event.trigger();

      /* Also call the separate response example handler. */
      ///////////////////res_separate.resume();
    }
#endif /* PLATFORM_HAS_BUTTON */
  }  /* while (1) */

  PROCESS_END();
}


#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

/* just for debug */
#pragma message(VAR_NAME_VALUE(UIP_ND6_SEND_RA))
#pragma message(VAR_NAME_VALUE(UIP_ND6_SEND_NS))
#pragma message(VAR_NAME_VALUE(UIP_ND6_SEND_NA))
#pragma message(VAR_NAME_VALUE(UIP_LLH_LEN))


