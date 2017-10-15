/*
 * config-loader.h
 *
 *  Created on: Oct 15, 2017
 *      Author: user
 */

#ifndef CONFIG_MANAGER_CONFIG_LOADER_H_
#define CONFIG_MANAGER_CONFIG_LOADER_H_

#define CONFIG_FLASH_OFFSET           0
#define CONFIG_MAGIC                  0xCC265002
// Load Errors
#define CONFIG_LOADER_LOAD_OK         0
#define CONFIG_LOADER_LOAD_OPEN_ERR   -1
#define CONFIG_LOADER_LOAD_READ_ERR   -2
#define CONFIG_LOADER_LOAD_MAGIC_NOK  -3
#define CONFIG_LOADER_LOAD_LEN_NOK    -4
/*---------------------------------------------------------------------------*/
/* Global configuration */
typedef struct tei_config_s {
  uint32_t magic;
  int len;
  uint16_t periodic_reads_rate;
  uint16_t max_consume_allowed;
} tei_config_t;

extern tei_config_t tei_configs;
/*---------------------------------------------------------------------------*/
/**
 * Loads the config from external flash and check if it's ok.
 *
 * \return < 0 if there is errors or 0 otherwise.
 */
int load_config();
/*---------------------------------------------------------------------------*/
/**
 * Saves the config to external flash.
 *
 * \return < 0 if there is errors or 0 otherwise.
 */
int save_config();
/*---------------------------------------------------------------------------*/

#endif /* CONFIG_MANAGER_CONFIG_LOADER_H_ */
