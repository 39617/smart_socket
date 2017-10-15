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

/*!< Event Read consume */
extern process_event_t read_consume_event;
/*!< */
extern uint16_t last_consume_read;

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
/*---------------------------------------------------------------------------*/
/**
 * Used to reset the last_consume_read and netctrl_node_data to zero
 */
void reset_reads();
/*---------------------------------------------------------------------------*/
PROCESS_NAME(consume_reader_process);

/**
 * @}
 */

#endif /* CONSUME_READER_H_ */
