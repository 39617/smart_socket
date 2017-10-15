/*
 * res-maxcons.c
 *
 *  Created on: Oct 15, 2017
 *      Author: user
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap-constants.h"

#include "include/smart-socket_constants.h"
#include "include/error_codes.h"
#include "consume-reader.h"
#include "smart_socket.h"

#define SHORT_MAX_LEN  5 /*!< 65535 */
#define MIN_VAL  0
#define MAX_VAL  15000


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
//

RESOURCE(res_maxcons,
		"title=\"Max Cons\"",
		res_get_handler,
		NULL,
		res_put_handler,
        NULL);

static char rsp_max_consume_as_json[] = "{\"max\":%hu}"; /*!< JSON object with the max consumption allowed */
//
static uint16_t parsed_max_cons;
/*---------------------------------------------------------------------------*/
static void set_bad_req(void *response) {
	REST.set_response_status(response, BAD_REQUEST_4_00);
	REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	int len = snprintf(error_buffer, ERROR_BUFFER_SIZE, error_template,
			error_invalid_params);
	REST.set_response_payload(response, error_buffer, len);
}
/*---------------------------------------------------------------------------*/
/**
 * @brief Parses the 'rate' query param
 * @return 1 if OK. 0 if can't convert.
 */
static int parse_max_cons_str(const char *rate_str) {
	parsed_max_cons = (uint16_t) strtoul(rate_str, NULL, 10);
	return (parsed_max_cons > 0 || parsed_max_cons < USHRT_MAX)? 1 : 0;
}
/*---------------------------------------------------------------------------*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	int len = snprintf((char *)buffer, MAX_RSP_PAYLOAD, rsp_max_consume_as_json, get_max_consume_allowed());
	REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
	REST.set_response_payload(response, buffer, len);
}
/*---------------------------------------------------------------------------*/
static void
res_put_handler(void *request, void *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset)
{
	char *_max_cons_str = NULL;
	int len;

	if((len = REST.get_query_variable(request, "m", (const char **)&_max_cons_str))) {
		if((len >0) && (len <= SHORT_MAX_LEN)) {
			*(_max_cons_str + len) = '\0';
			int res = parse_max_cons_str((const char *)_max_cons_str);
			if((res == 0) || (parsed_max_cons < MIN_VAL) || (parsed_max_cons > MAX_VAL)) {
				set_bad_req(response);
				return;
			} else {
				// Set the new max value
				update_max_consume_allowed(parsed_max_cons);
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
