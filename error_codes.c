/**
 * @file     error_codes.c
 * @brief    Contains the information error used for the API REST
 * @version  1.0
 * @date     01 Jun. 2017
 * @author   Tiago Costa & Ricardo Jesus & Claudio Prates
 *
 **/

#include "include/error_codes.h"

char error_buffer[ERROR_BUFFER_SIZE]; /*!< Error Buffer */
char error_template[] = "{\"error\":\"%s\"}"; /*!< Template */

char error_invalid_params[] =           "0x101"; /*!< Invalid parameters */
