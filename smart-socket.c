#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
//
#include "ti-lib.h"
#include "leds.h"
//
#include "netctrl-client.h"
#include "netctrl-platform.h"
//
#include "consume-reader.h"

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

#define SENSORS_CONFIG_CHANNEL(sensor, channel) (sensor).configure(ADC_SENSOR_SET_CHANNEL, channel)

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
  res_switch,
  res_readcons;

/* Used to store data to be sent over netctrl */
uint32_t netctrl_node_data = 0x61626364;

/** IP's Controller */
uip_ipaddr_t controller_ipaddr = {
		.u16[0] = 0x80fe,
		.u16[1] = 0x0,
		.u16[2] = 0x0,
		.u16[3] = 0x0,
		.u16[4] = 0x1200,
		.u16[5] = 0xff4b,
		.u16[6] = 0x27fe,
		.u16[7] = 0x0fc5 };

PROCESS(smart_socket, "Smart-Socket");
AUTOSTART_PROCESSES(&smart_socket);


/**
 * Sets the IP configuration for the RF interface.
 */
static void init_rf_if_addr(void) {
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

#define LIGHT_ON_DURATION   0.05
#define LIGHT_OFF_DURATION  5
static clock_time_t update_light_signal() {
  static uint8_t state;

  if(state == 0) {
    state = 1;
    leds_on(LEDS_RED);
    return LIGHT_ON_DURATION * CLOCK_SECOND;
  }

  state = 0;
  leds_off(LEDS_RED);
  return LIGHT_OFF_DURATION * CLOCK_SECOND;
}


PROCESS_THREAD(smart_socket, ev, data)
{
  static struct etimer et_light_signal;
  static struct etimer et_netctrl;

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
  rest_activate_resource(&res_switch, "switch"); // on/off switch
  rest_activate_resource(&res_readcons, "readcons"); // read energetic consume

  // Netctrl
  netctrl_client_init_network(&controller_ipaddr, NETCTRL_DEFAULT_LISTEN_PORT);

  /* Init Consume Reader */
  init_consume_reader();

  etimer_set(&et_light_signal, update_light_signal());
  etimer_set(&et_netctrl, 5 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();
#if PLATFORM_HAS_BUTTON
    if(ev == sensors_event && data == &button_sensor) {
      PRINTF("*******BUTTON*******\n");
      PRINTF("ADC Value: %d\n", read_consumption());

      // TODO: Ver isto do res_event.trigger - serve para os updates periodicos dos consumos
      /* Call the event_handler for this application-specific event. */
      //////////////res_event.trigger();

      /* Also call the separate response example handler. */
      ///////////////////res_separate.resume();
    } else
#endif /* PLATFORM_HAS_BUTTON */
    if(ev == PROCESS_EVENT_TIMER && data == &et_light_signal) {
    	etimer_set(&et_light_signal, update_light_signal());
    } else if(ev == PROCESS_EVENT_TIMER && data == &et_netctrl) {
		etimer_set(&et_netctrl,
				netctrl_client_handle_event(NETCTRL_CLIENT_EVT_TIMER) * CLOCK_SECOND);
	} else if(ev == tcpip_event) {
		if(uip_newdata()) {
			PRINTF("** Receiving UDP datagram from: ");
			PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
			PRINTF(":%u\n  Length: %u\n", uip_ntohs(UIP_UDP_BUF->srcport),
				   uip_datalen());
			//
			etimer_set(&et_netctrl,
					netctrl_client_handle_event(NETCTRL_CLIENT_EVT_NET) * CLOCK_SECOND);
		}
	}
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


