/**
 * \addtogroup consume-reader
 * @{
 */

/*
 * @file     consume-reader.c
 * @brief    Read the consume through the ADC.
 * @version  0.1
 * @author   Claudio Prates & Ricardo Jesus & Tiago Costa
 */

#include "consume-reader.h"
//
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "contiki-net.h"
//
#include "rest-engine.h"
#include "er-coap-engine.h"
#include "include/smart-socket_constants.h"


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


/* Used to store the last consume readed. */
uint16_t last_consume_read;
/*!< Event Read consume */
process_event_t read_consume_event;
//
static coap_packet_t request_packet[1]; /*!< Used to send CoAP messages */
char rsp_consume_read_as_json[] = "{\"cons\":%d}"; /*!< JSON object with the consumption read */

/** IP's Controller */
extern uip_ipaddr_t controller_ipaddr;

PROCESS(consume_reader_process, "Consume-Reader");

/*---------------------------------------------------------------------------*/
void init_consume_reader() {
	SENSORS_ACTIVATE(adc_sensor);
	// use default channel
	//SENSORS_CONFIG_CHANNEL(adc_sensor, ADC_COMPB_IN_AUXIO0);
	process_start(&consume_reader_process, NULL);
}
/*---------------------------------------------------------------------------*/
int read_consumption() {
	return adc_sensor.value(ADC_SENSOR_VALUE);
}
/*---------------------------------------------------------------------------*/
static void prepare_request()
{
	static char buff[20];
    /* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
    coap_init_message(request_packet, COAP_TYPE_CON, METHOD_POST, 0);
    coap_set_header_uri_path(request_packet, COAPTOHTTP_URI);
    coap_set_header_uri_query(request_packet, CONSUME_PATH);
    int len = snprintf((char *)buff, MAX_RSP_PAYLOAD, rsp_consume_read_as_json, last_consume_read);
	coap_set_payload(request_packet, (uint8_t *)&buff, len);
}

/**
 * @brief Handles chunks transfer
 *
 * Takes the chunk received and copies to original_request buffer
 *
 * @param response : Pointer to the response packet
 * @return nothing
 */
void client_chunk_handler(void *response)
{
	PRINTF("CoAP response received\n");
	// TODO: handle the response
    coap_packet_t * res = (coap_packet_t *) response;
    const uint8_t *chunk;
    // select CoAP rest
    rest_select_if(COAP_IF);
    // Status code
    PRINTF("Status code: %d\n",  res->code);
    int len = REST.get_request_payload(response, &chunk);
    PRINTF("Payload len: %d\n",  len);
}

PROCESS_THREAD(consume_reader_process, ev, data)
{
  PROCESS_BEGIN();

  // Get an Event Id
  read_consume_event = process_alloc_event();

  PROCESS_PAUSE();

  PRINTF("* Starting Consume Reader\n");

  while(1) {
    PROCESS_WAIT_EVENT();

	if(ev == read_consume_event) {
      PRINTF("   Periodic Read Event\n");
      last_consume_read = read_consumption();
      PRINTF("   ADC Value: %d\n", last_consume_read);
      prepare_request();
      // Make the CoAP request!
	  COAP_BLOCKING_REQUEST(
			  &controller_ipaddr,
			  UIP_HTONS(COAP_DEFAULT_PORT), request_packet,
			  client_chunk_handler);
    }
  }  /* while (1) */

  PROCESS_END();
}

/**
 * @}
 */
