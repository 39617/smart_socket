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
#include "smart_socket.h"
#include "netctrl-client.h"
#include "netctrl-platform.h"
#include "include/smart-socket_constants.h"
//
#include "consume-reader.h"
//
#include "dev/button-sensor.h"
//
#include "config-loader.h"


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
#define PEIODIC_READS_DEFAULT_RATE  5
#define DEFAULT_MAX_CONS_ALLOWED  2000

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
  res_readcons,
  res_readrate,
  res_maxcons;

extern int consume_reader_requests;
// used to stop periodic reads when the switch is off
extern uint8_t switch_state;

/* Used to store data to be sent over netctrl */
uint32_t netctrl_node_data = 0x0;

/** Periodic readings timer */
static struct etimer et_periodic_read;
/** Specifies the readings rate - In Seconds */
static uint16_t tei_reading_rate = PEIODIC_READS_DEFAULT_RATE;
/** Max allowed consumption on this TEI */
uint16_t max_cons_allowed = DEFAULT_MAX_CONS_ALLOWED;

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
/*---------------------------------------------------------------------------*/
void update_readings_rate(uint16_t rate) {
  tei_reading_rate = rate;
  etimer_stop(&et_periodic_read);
  etimer_set(&et_periodic_read, tei_reading_rate * CLOCK_SECOND);
  // Save the new Config
  save_config();
}
/*---------------------------------------------------------------------------*/
void update_max_consume_allowed(uint16_t consume) {
	max_cons_allowed = consume;
  // Updates the new Config
  save_config();
}
/*---------------------------------------------------------------------------*/
uint16_t get_max_consume_allowed() {
	return max_cons_allowed;
}
/*---------------------------------------------------------------------------*/
uint16_t get_readings_rate() {
	return tei_reading_rate;
}
/*---------------------------------------------------------------------------*/
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

  // Load Configs
  PRINTF("Loading configs...\n");
  int ret = load_config();
  if(ret != CONFIG_LOADER_LOAD_OK) {
    PRINTF("Loading configs failed...\n");
    // May be the first time
    save_config();
  } else {
	  tei_reading_rate = tei_configs.periodic_reads_rate;
	  update_max_consume_allowed(tei_configs.max_consume_allowed);
  }
  PRINTF("Loadig done.\n");

  /* Initialize the REST engine. */
  // TODO: ...
  http_rest_implementation.init = http_init_engine;
  http_rest_implementation.set_service_callback = http_set_service_callback;
  rest_init_engine();

  /* Activate resources */
  rest_activate_resource(&res_switch, "switch"); // on/off switch
  rest_activate_resource(&res_readcons, "readcons"); // read energetic consume
  rest_activate_resource(&res_readrate, "readrate"); // used to configure periodic reads
  rest_activate_resource(&res_maxcons, "maxcons"); // used to configure max consumption allowed

  // Netctrl
  netctrl_client_init_network(&controller_ipaddr, NETCTRL_DEFAULT_LISTEN_PORT);

  /* Init Consume Reader */
  init_consume_reader();

  etimer_set(&et_light_signal, update_light_signal());
  etimer_set(&et_netctrl, 1 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT();

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
			uint8_t registered = netctrl_is_registered();
			// Handle the response
			etimer_set(&et_netctrl,
					netctrl_client_handle_event(NETCTRL_CLIENT_EVT_NET) * CLOCK_SECOND);
			// Start periodic reads only after a successfully registration
			if((registered != NETCTRL_CLIENT_REGISTERED) && netctrl_is_registered()
					&& etimer_expired(&et_periodic_read)) {
				etimer_set(&et_periodic_read, 1 * CLOCK_SECOND);
			}
		}
	} else if(((ev == PROCESS_EVENT_TIMER && data == &et_periodic_read)
    		|| (ev == sensors_event && data == &button_sensor))
			&& switch_state != SWITCH_STATE_OFF && netctrl_is_registered()) {
      // Send a periodic read request
      if(consume_reader_requests == 0) {
    	  process_post(&consume_reader_process, read_consume_event, NULL);
      }
      etimer_set(&et_periodic_read, 1 * CLOCK_SECOND);
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
