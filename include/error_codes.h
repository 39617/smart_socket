/**
 * @file     error_codes.h
 * @brief    Contains the definition of all error used for the API REST and expose it
 * @version  1.0
 * @date     01 Jun. 2017
 * @author   Tiago Costa & Ricardo Jesus & Claudio Prates
 *
 **/

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_

/** @defgroup CoAP CoAP
* @{
*/

/** @addtogroup Error
* @{
*/

#define ERROR_BUFFER_SIZE                     20 /*!< Defines the error buffer size */

extern char error_buffer[ERROR_BUFFER_SIZE]; /*!< Error Buffer */
extern char error_template[]; /*!< Template */

extern char error_invalid_params[]; /*!< Invalid parameters */
extern char error_node_not_found[]; /*!< Node offline */
extern char error_node_unavailable[]; /*!< Max requests per node reached */
extern char error_too_many_requests[]; /*!< Too Many Requests */

/**
 * @}
 */

/**
 * @}
 */

#endif /* ERROR_CODES_H_ */
