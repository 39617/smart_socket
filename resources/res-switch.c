/**
 * res-switch.c
 *
 * This Resource is responsible for handling the Smart-Socket's switch.
 *
 * - Resource name: switch
 * - Methods
 *   - GET - return the switch's state
 *     - Parameter
 *       - NONE
 *
 *   - PUT - set the switch's state
 *     - Parameter
 *       - s
 *         - Mandatory. Can take tow values: 'on' and 'off' that turn the Smart-Socket On or Off respectively.
 *
 * Usage
 *  - http://[fec0::a]/coapnode?t=1234567890&a=switch&p=s=on
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap-constants.h"

#include "include/smart-socket_constants.h"
#include "include/error_codes.h"
#include "consume-reader.h"

#include "leds.h"


#define MAX_ETAG_SIZE  2
#define NODE_DATA_SWITCH_SHIFT  16
#define NODE_DATA_SWITCH_MASK   (0x1 << NODE_DATA_SWITCH_SHIFT)


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_switch,
		"title=\"Switch\"",
		res_get_handler,
		NULL,
		res_put_handler,
        NULL);

uint8_t switch_state = SWITCH_DEFAULT_STATE;
static char rsp_switch_state_as_json[] = "{\"state\":%u}";
/**
 * eTAG to control the version of the switch's state
 */
static uint8_t validate_etag[MAX_ETAG_SIZE] = { 0 };
static uint8_t validate_etag_len = 1;
static uint8_t validate_change = 1;
/* Used to store data to be sent over netctrl */
extern uint32_t netctrl_node_data;

/*---------------------------------------------------------------------------*/
static void
validate_update_etag()
{
  int i;
  validate_etag_len = (random_rand() % MAX_ETAG_SIZE) + 1;
  for(i = 0; i < validate_etag_len; ++i) {
    validate_etag[i] = random_rand();
  }
  validate_change = 0;

  // TODO: Test etag
  //printf("### SERVER ACTION ### Changed ETag %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n",
  //       validate_etag_len, validate_etag[0], validate_etag[1], validate_etag[2], validate_etag[3], validate_etag[4], validate_etag[5], validate_etag[6], validate_etag[7]);
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	if(validate_change) {
		validate_update_etag();
	}

	int len = snprintf((char *)buffer, MAX_RSP_PAYLOAD,
			rsp_switch_state_as_json, switch_state);

	REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	REST.set_header_etag(response, validate_etag, validate_etag_len);
	REST.set_response_payload(response, buffer, len);
}

static void set_bad_req(void *response) {
	REST.set_response_status(response, BAD_REQUEST_4_00);
	REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	int len = snprintf(error_buffer, ERROR_BUFFER_SIZE, error_template,
			error_invalid_params);
	REST.set_response_payload(response, error_buffer, len);
}
/*---------------------------------------------------------------------------*/
static void
res_put_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	const char *state = NULL;
	int len;

	if((len = REST.get_query_variable(request, "s", &state))) {
		if((strncmp(state, "on", 2) == 0) && (len == 2)) {
			// TODO: passa a trabalhar com um driver
			leds_on(LEDS_GREEN);
			switch_state = SWITCH_STATE_ON;
		}
		else if ((strncmp(state, "off", 3) == 0) && (len == 3)) {
			leds_off(LEDS_GREEN);
			switch_state = SWITCH_STATE_OFF;
			// reset reads
			reset_reads();
		}
		else {
			set_bad_req(response);
			return;
		}

		netctrl_node_data = (netctrl_node_data & (~NODE_DATA_SWITCH_MASK))
				| ((switch_state != SWITCH_STATE_OFF) << NODE_DATA_SWITCH_SHIFT);
	} else {
		set_bad_req(response);
		return;
	}

	/* Set resource as changed */
	validate_change = 1;
	REST.set_response_status(response, CHANGED_2_04);
}

