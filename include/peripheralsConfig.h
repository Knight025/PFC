/*
 * peripherals.h
 *
 *  Created on: 20/6/2016
 *      Author: Santiago Real
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_


void init_timers();
void EPWM_initEpwm();
void ADC_initAdcA();
void initDAC();
void LED_configure();
void LED_on();
void LED_off();



#endif /* PERIPHERALS_H_ */
