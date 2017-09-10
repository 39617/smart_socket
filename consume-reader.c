/**
 * \addtogroup consume-reader
 * @{
 */

/*
 * @file     consume-reader.c
 * @brief    Read the consume through the ADC.
 * @version  0.1
 * @author   Claudio Prates & Ricardo Jesus & Tiago Costa
 */

#include "consume-reader.h"

/*---------------------------------------------------------------------------*/
void init_consume_reader() {
	SENSORS_ACTIVATE(adc_sensor);
	// use default channel
	//SENSORS_CONFIG_CHANNEL(adc_sensor, ADC_COMPB_IN_AUXIO0);
}
/*---------------------------------------------------------------------------*/
int read_consumption() {
	return adc_sensor.value(ADC_SENSOR_VALUE);
}

/**
 * @}
 */
