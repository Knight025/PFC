/*
 * CLA_Commands.h
 *
 *  Created on: 11/1/2017
 *      Author: Santiago Real
 */

/* This header purpose is to avoid including other dsp_shared.h
 * variables anywhere in C28 code
*/

#ifndef INCLUDE_COMMON_DEFINES_H_
#define INCLUDE_COMMON_DEFINES_H_

#define SIGNAL_LENGTH				100		// Longitud maxima de cada pulso en muestras (NOTA: DEBE SER > 64)
#define ADC_PERIOD					1		// Period in us
#define DAC_ZERO					2048	// Bipolar signal, DAC 12 bits -> (2^12/2) = 2^11 = 2048

//Mode designed to measure CLA->C28->CLA latency with TIMER1
#define DEBUG_LATENCY				0


#endif /* INCLUDE_COMMON_DEFINES_H_ */
