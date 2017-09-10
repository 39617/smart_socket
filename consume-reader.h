#ifndef CONSUME_READER_H_
#define CONSUME_READER_H_

/**
 * \defgroup consume-reader
 * @{
 */

/*
 * @file     consume-reader.h
 * @brief    Read the consume through the ADC.
 * @version  0.1
 * @author   Claudio Prates & Ricardo Jesus & Tiago Costa
 */

#include "dev/adc-sensor.h"

/*---------------------------------------------------------------------------*/
/**
 * Initialize the Consume Reader
 */
void init_consume_reader();
/*---------------------------------------------------------------------------*/
/**
 * Read the current consume through the ADC.
 *
 * \return Returns the calibrated value based on the ADC read.
 */
int read_consumption();

/**
 * @}
 */

#endif /* CONSUME_READER_H_ */
