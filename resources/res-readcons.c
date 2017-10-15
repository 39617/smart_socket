/**
 * res-readcons.c
 *
 * This Resource is responsible for handling the Smart-Socket's ADC reads.
 *
 * - Resource name: readcons
 * - Methods
 *   - GET - return the ADC read
 *     - Parameter
 *       - NONE
 *
 * Usage
 *  - http://[fec0::a]/coapnode?t=1234567890&a=readcons
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap-constants.h"

#include "include/smart-socket_constants.h"
#include "include/error_codes.h"
#include "consume-reader.h"



static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
//
extern uint8_t switch_state;

RESOURCE(res_readcons,
		"title=\"Consume Reader\"",
		res_get_handler,
		NULL,
		NULL,
        NULL);

extern char rsp_consume_read_as_json[];
/*---------------------------------------------------------------------------*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	int reading;
	if(switch_state == SWITCH_STATE_ON) {
		reading = read_consumption();
	} else {
		reading = 0;
	}
	int len = snprintf((char *)buffer, MAX_RSP_PAYLOAD, rsp_consume_read_as_json, reading);
	REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	REST.set_response_payload(response, buffer, len);
}
/*---------------------------------------------------------------------------*/
