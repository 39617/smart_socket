/*
 * smart_socket.h
 *
 *  Created on: Oct 15, 2017
 *      Author: user
 */
#ifndef SMART_SOCKET_H_
#define SMART_SOCKET_H_

#include "contiki.h"

PROCESS_NAME(smart_socket);

uint16_t get_readings_rate();
/** */
void update_max_consume_allowed(uint16_t consume);
/** */
uint16_t get_max_consume_allowed();
/** Max allowed consumption on this TEI */
extern uint16_t max_cons_allowed;

#endif /* SMART_SOCKET_H_ */
