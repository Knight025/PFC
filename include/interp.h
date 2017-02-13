/*
 * interp.h
 *
 *  Created on: 22/6/2016
 *      Author: Santiago Real
 */

#ifndef INTERP_H_
#define INTERP_H_


struct INDIVDATA {
	uint16_t data1;
	uint16_t data2;
	uint16_t data3;
	uint16_t data4;
	uint16_t data5;
};


union SDATA {
	uint16_t dataArray[5];
	struct INDIVDATA data;
};



struct PRUEBA {                  // bits description
	char command;
    union SDATA args;
};



int execute_pkg ();
int interp_addcmd ();


#endif /* INTERP_H_ */
