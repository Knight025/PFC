/*
 * serial.h
 *
 *  Created on: 20/6/2016
 *      Author: Santiago Real
 */

#ifndef SERIAL_H_
#define SERIAL_H_

// At first I used 1024 bytes length, but as I want 'playList' to fit in only one package,
// I changed it to 8100 bytes length (for now, playList max lenth = 4000 uint16 = 8000 bytes,
// so when I wrote this note, max package length = 2 + 2 + 8000 = 8004)
#define SCI_BUFF_LENGTH		8192	//LENGTH (BYTES). IT HAS TO BE MULTIPLE OF 2!!



struct SCI_PKG {
	uint16_t length;
	uint16_t commandID;
	uint16_t* data;
};


struct SCI_PKG *getPkg();
void scia_init();
void scia_xmit();
void scia_msg();
void scia_pkg();
int receive_pkg();


#endif /* SERIAL_H_ */
