/**
 * res-readrate.c
 *
 * This Resource is responsible for handling the readings rate.
 *
 * - Resource name: readrate
 * - Methods
 *   - GET - return the readings rate
 *     - Parameter
 *       - NONE
 *
 *   - PUT - set the readings rate
 *     - Parameter
 *       - r
 *         - Mandatory. Can take values from 5 to 65535. In seconds
 *
 * Usage
 *  - http://[fec0::a]/readrate?r=30
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "rest-engine.h"
#include "er-coap-constants.h"

#include "include/smart-socket_constants.h"
#include "include/error_codes.h"
#include "smart_socket.h"

#define SHORT_MAX_LEN  5 /*!< 65535 */
#define MIN_RATE  5
#define MAX_RATE  65535

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_readrate,
		"title=\"ReadRate\"",
		res_get_handler,
		NULL,
		res_put_handler,
        NULL);

static char rsp_read_rate_as_json[] = "{\"rate\":%hu}";
static uint16_t rate;

extern void update_readings_rate(uint16_t rate);
extern uint16_t get_readings_rate();
/*---------------------------------------------------------------------------*/
/**
 * @brief Parses the 'rate' query param
 * @return 1 if OK. 0 if can't convert.
 */
static int parse_rate_str(const char *rate_str) {
	rate = (uint16_t) strtoul(rate_str, NULL, 10);
	return (rate > 0 || rate < USHRT_MAX)? 1 : 0;
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	int len = snprintf((char *)buffer, MAX_RSP_PAYLOAD,
			rsp_read_rate_as_json, get_readings_rate());

	REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	// TODO: see res-switch for more information
	//REST.set_header_etag(response, validate_etag, validate_etag_len);
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
	char *_rate_str = NULL;
	int len;

	if((len = REST.get_query_variable(request, "r", (const char **)&_rate_str))) {
		if((len >0) && (len <= SHORT_MAX_LEN)) {
			*(_rate_str + len) = '\0';
			int res = parse_rate_str((const char *)_rate_str);
			if((res == 0) || (rate < MIN_RATE) || (rate > MAX_RATE)) {
				set_bad_req(response);
				return;
			} else {
				// Set the new rate
				// We need to change to the right process context to update the timer
				PROCESS_CONTEXT_BEGIN(&smart_socket);
				update_readings_rate(rate);
				PROCESS_CONTEXT_END(&smart_socket);
			}
		} else {
			set_bad_req(response);
			return;
		}
	} else {
		set_bad_req(response);
		return;
	}

	REST.set_response_status(response, CHANGED_2_04);
}
