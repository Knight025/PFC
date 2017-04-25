/*
 * serial.c
 *
 *  Created on: 20/6/2016
 *      Author: Santiago Real (based on TI's firmware code)
 */
#include "F28x_Project.h"
#include "serial.h"
#include <string.h>



uint16_t counter = 0;
uint16_t pkg_length = 0;

#pragma DATA_SECTION(sciInputBuffer, "ramgs0");
uint16_t sciInputBuffer[SCI_BUFF_LENGTH/2];
struct SCI_PKG pkg;


struct SCI_PKG *getPkg(){
	return &pkg;
}



// Test 1,SCIA  DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity
void scia_init(uint32_t baudRate){
	uint32_t BRR = 50000000/(baudRate*8 -1);
	// Note: Clocks were turned on to the SCIA peripheral
	// in the InitSysCtrl() function

	// (Santi) las siguientes 3 lineas de codigo constituian una funcion aparte,
	// scia_fifo_init()
	// Initialize the SCI FIFO:
	SciaRegs.SCIFFTX.all = 0xE040;
	SciaRegs.SCIFFRX.all = 0x2044;
	SciaRegs.SCIFFCT.all = 0x0;



 	SciaRegs.SCICCR.all = 0x0007;   // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // async mode, idle-line protocol
	SciaRegs.SCICTL1.all = 0x0003;  // enable TX, RX, internal SCICLK,
                                   // Disable RX ERR, SLEEP, TXWAKE
	SciaRegs.SCICTL2.all = 0x0003;
	SciaRegs.SCICTL2.bit.TXINTENA = 1;
	SciaRegs.SCICTL2.bit.RXBKINTENA = 1;

	//
	// SCIA at 9600 baud
	// @LSPCLK = 50 MHz (200 MHz SYSCLK) HBAUD = 0x02 and LBAUD = 0x8B.
	// @LSPCLK = 30 MHz (120 MHz SYSCLK) HBAUD = 0x01 and LBAUD = 0x86.

	//SciaRegs.SCIHBAUD.all = 0x0002;	Esto es para 9600 Baud
	//SciaRegs.SCILBAUD.all = 0x008B;

	SciaRegs.SCIHBAUD.all = (0xFF00 & BRR)>>8;
	SciaRegs.SCILBAUD.all = 0x00FF & BRR;

	SciaRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
}

// Transmit a character from the SCI
void scia_xmit(uint16_t a){
	while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
	SciaRegs.SCITXBUF.bit.TXDT = ((0xFF00)&a)>>8;
	while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
	SciaRegs.SCITXBUF.bit.TXDT = 0x00FF&a;
}

void scia_pkg(uint16_t * msg, uint16_t length){
	int i;
	scia_xmit(2*length+2);
	for(i = 0; i < length; i++){
		scia_xmit(msg[i]);
	}
}

void scia_msg(char* msg, int partitioned){
	int i = 0;
	if(partitioned == 0)
		scia_xmit(strlen(msg)+2);

	while(msg[i] != '\0'){
		while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
		SciaRegs.SCITXBUF.bit.TXDT = 0x00FF & msg[i++];
	}

}



// Receives sci pkg bit by bit. It is intended to be used as follow: while(receive_pkg < 1) { };
int receive_pkg(void){
	if(SciaRegs.SCIFFRX.bit.RXFFST == 0){ // wait for XRDY = 1 for empty state
		return 0;
	}
	else{
		if(counter >= SCI_BUFF_LENGTH)
			return -1;

		if(counter%2){
			sciInputBuffer[(counter-1)/2] |= SciaRegs.SCIRXBUF.bit.SAR;
		}else{
			sciInputBuffer[counter/2] = 0;
			sciInputBuffer[counter/2] |= SciaRegs.SCIRXBUF.bit.SAR<<8;
		}
		counter++;

		//sciInputBuffer[counter++] = SciaRegs.SCIRXBUF.bit.SAR;

		if(counter == 2)
			pkg_length = sciInputBuffer[0];

		if (counter == pkg_length){
			pkg.length = sciInputBuffer[0];
			pkg.commandID = pkg_length = sciInputBuffer[1];
			pkg.data = &sciInputBuffer[2];
			pkg_length = 0;
			counter = 0;
			return 1;
		}
	}

	return 0;
}



