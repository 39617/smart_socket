/*
 * config-loader.c
 *
 *  Created on: Oct 15, 2017
 *      Author: user
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config-loader.h"
#include "smart_socket.h"
#include "board-peripherals.h"


#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


tei_config_t tei_configs;

/*---------------------------------------------------------------------------*/
int
save_config()
{
  /* Dump current running config to flash */
  int rv;
  int ret_val;

  rv = ext_flash_open();

  if(!rv) {
    PRINTF("!!! Could not open flash to save config\n");
    ext_flash_close();
    ret_val = -1;
  }

  rv = ext_flash_erase(CONFIG_FLASH_OFFSET, sizeof(tei_config_t));

  if(!rv) {
	  PRINTF("!!! Error erasing flash\n");
	  ret_val = -2;
  } else {
    // Save configs
	tei_configs.magic = CONFIG_MAGIC;
	tei_configs.len = sizeof(tei_config_t);
	tei_configs.periodic_reads_rate = get_readings_rate();

    rv = ext_flash_write(CONFIG_FLASH_OFFSET, sizeof(tei_config_t),
                         (uint8_t *)&tei_configs);
    if(!rv) {
    	PRINTF("!!! Error saving config\n");
    	ret_val = -3;
    } else {
    	ret_val = 0;
    }
  }

  ext_flash_close();
  return ret_val;
}
/*---------------------------------------------------------------------------*/
int
load_config()
{
  /* Read from flash into a temp buffer */
  static tei_config_t tmp_cfg;

  int rv = ext_flash_open();

  if(!rv) {
    PRINTF("!!! Could not open flash to load config\n");
    ext_flash_close();
    return CONFIG_LOADER_LOAD_OPEN_ERR;
  }

  rv = ext_flash_read(CONFIG_FLASH_OFFSET, sizeof(tmp_cfg),
                      (uint8_t *)&tmp_cfg);

  ext_flash_close();

  if(!rv) {
    PRINTF("Error loading config\n");
    return CONFIG_LOADER_LOAD_READ_ERR;
  }

  if(tmp_cfg.magic == CONFIG_MAGIC) {
    if(tmp_cfg.len == sizeof(tmp_cfg)) {
      memcpy(&tei_configs, &tmp_cfg, sizeof(tei_configs));
    } else {
      return CONFIG_LOADER_LOAD_LEN_NOK;
    }
  } else {
    return CONFIG_LOADER_LOAD_MAGIC_NOK;
  }

  return CONFIG_LOADER_LOAD_OK;
}
/*---------------------------------------------------------------------------*/
