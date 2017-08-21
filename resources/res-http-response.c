/**
 * res-http-response.c
 *
 * This Resource is responsible for handling the Smart-Socket's response after it requested a http operation
 *
 */

#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "er-coap-constants.h"

#include "include/smart-socket_constants.h"
#include "include/error_codes.h"


static void res_common_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_http_response,
		"title=\"HttpResponse\"",
		res_common_handler,
		res_common_handler,
		res_common_handler,
		res_common_handler);


static void
res_common_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	REST.set_response_status(response, CONTENT_2_05);
}

