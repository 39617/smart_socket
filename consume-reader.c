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
#include <float.h>
#include "contiki.h"
#include "contiki-net.h"
//
#include "rest-engine.h"
#include "er-coap-engine.h"
#include "leds.h"
#include "include/smart-socket_constants.h"
#include "smart_socket.h"


#define DEBUG 0
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

#define NODE_DATA_MASK  0xFFFF
#define SAMPLE_DURATION  0.5f

/* Used to store the last consume readed. */
uint16_t last_consume_read;
/*!< Event Read consume */
process_event_t read_consume_event;
//
static coap_packet_t request_packet[1]; /*!< Used to send CoAP messages */
char rsp_consume_read_as_json[] = "{\"cons\":%d}"; /*!< JSON object with the consumption read */
/** IP's Controller */
extern uip_ipaddr_t controller_ipaddr;
/* Used to store data to be sent over netctrl */
extern uint32_t netctrl_node_data;
//
static uint16_t events_counter = 0;

/**
 * Number of requests
 */
int consume_reader_requests = 0;



int mVperAmp = 66; // use 100 for 20A Module and 66 for 30A Module
static float Voltage = 0;
static float VRMS = 0;
static float AmpsRMS = 0;
static int get_vpp(float *value);




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
	if(get_vpp(&Voltage)) {
		return 1;
	}

	VRMS = (Voltage/2.0f) * 0.707f;  //root 2 is 0.707
	AmpsRMS = (VRMS * 1000)/mVperAmp;
	last_consume_read = (int) ((AmpsRMS-0.098f) * 1000.0f);

	return 0;
}
/*---------------------------------------------------------------------------*/
void reset_reads() {
	last_consume_read = 0;
	netctrl_node_data = (netctrl_node_data & (~NODE_DATA_MASK)) | last_consume_read;
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
/*---------------------------------------------------------------------------*/
/**
 * @brief Handles chunks transfer
 *
 * Takes the chunk received and copies to original_request buffer
 *
 * @param response : Pointer to the response packet
 * @return nothing
 */
static void client_chunk_handler(void *response)
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
/*---------------------------------------------------------------------------*/
static float maxValue = 0;          // store max value here
static float minValue = FLT_MAX;          // store min value here
static clock_time_t start_time;
static int get_vpp(float *value)
{
  float readValue;


   if((clock_time()-start_time) < (CLOCK_SECOND * SAMPLE_DURATION)) //sample for 0.2 Sec
   {
       readValue = adc_sensor.value(ADC_SENSOR_VALUE) / 1000000.0f;
       // see if you have a new maxValue
       if (readValue > maxValue)
       {
           /*record the maximum sensor value*/
           maxValue += readValue;
           maxValue = maxValue / 2.0f;
       }
       else if (readValue < minValue)
       {
           /*record the minimum sensor value*/
           minValue += readValue;
           minValue = minValue / 2.0f;
       }

       *value = (maxValue - minValue);
       return 1;
   } else {
	   maxValue = 0;
	   minValue = FLT_MAX;
	   return 0;
   }
 }
/*---------------------------------------------------------------------------*/
static int max_consume_allowed_ok() {
  return max_cons_allowed - last_consume_read;
}
/*---------------------------------------------------------------------------*/
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
      start_time = clock_time();
      process_poll(&consume_reader_process);
      read_consumption();
      netctrl_node_data = (netctrl_node_data & (~NODE_DATA_MASK)) | last_consume_read;
      PRINTF("   ADC Value: %d\n", last_consume_read);
      // Check max consumption
      if(max_consume_allowed_ok() < 0) {
    	  leds_off(LEDS_GREEN);
      }

      if(events_counter >= get_readings_rate()) {
		  prepare_request();
		  // Make the CoAP request!
		  consume_reader_requests++;
		  COAP_BLOCKING_REQUEST(
				  &controller_ipaddr,
				  UIP_HTONS(COAP_DEFAULT_PORT), request_packet,
				  client_chunk_handler);
		  consume_reader_requests--;
		  events_counter = 0;
      } else {
    	  events_counter ++;
      }
    } else if(ev == PROCESS_EVENT_POLL) {
    	if(read_consumption()) {
    		process_poll(&consume_reader_process);
    	}
    }
  }  /* while (1) */

  PROCESS_END();
}
/**
 * @}
 */
