/*
 * smart-socket_constants.h
 *
 */

#ifndef INCLUDES_SMART_SOCKET_CONSTANTS_H_
#define INCLUDES_SMART_SOCKET_CONSTANTS_H_

#define MAX_RSP_PAYLOAD 							64 + 1			/*!< +1 for the terminating zero, which is not transmitted */

#define SWITCH_DEFAULT_STATE						0				/*!< Default state of the Smart-Socket's switch */
#define SWITCH_STATE_ON                             0x1            /*!< Switch state ON */
#define SWITCH_STATE_OFF                            0x0            /*!< Switch state OFF */


#endif /* INCLUDES_SMART_SOCKET_CONSTANTS_H_ */
