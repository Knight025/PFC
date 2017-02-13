/*
 * CLA_Command_Int.c
 *
 *  Created on: 11/1/2017
 *      Author: Santiago Real
 */

#include <Common_Defines.h>

#include "F28x_Project.h"
#include "dsp_shared.h"
#include "CLA_Command_Int.h"


bool ACKReceived = 0;


//Saves CLA 'volt', 'volt2' arrays in one buffer of minimum length 'SIGNAL_LENGTH'
void CLA_readOutputBuff(uint16_t* buff){
	uint16_t* pointer = buff;	//Innecesary, but just to be sure...
	int i;
	for (i = 0; i < SIGNAL_LENGTH_ABUFF; i++){
		*pointer++ = volt[i];
	}
	for (i = 0; i < SIGNAL_LENGTH_BBUFF; i++){
		*pointer++ = volt2[i];
	}
}

//Saves an array of length 'SIGNAL_LENGTH' in CLA's input buffers 'signalBuff', 'signalBuff2'
void CLA_writeInputBuff(uint16_t* buff){
	uint16_t* pointer = buff;
	int i;
	for (i = 0; i < SIGNAL_LENGTH_ABUFF; i++){
		signalBuff[i] = *pointer++;
	}
	for (i = 0; i < SIGNAL_LENGTH_BBUFF; i++){
		signalBuff2[i] = *pointer++;
	}
}

void CLA_clearInputBuff(){
	int i;
	for (i = 0; i < SIGNAL_LENGTH_ABUFF; i++){
		signalBuff[i] = DAC_ZERO;
	}
	for (i = 0; i < SIGNAL_LENGTH_BBUFF; i++){
		signalBuff2[i] = DAC_ZERO;
	}
}


void CLA_requestFeedback(){
	feedbackReq = 1;
}

void CLA_waitACK(){
	while (!ACKReceived){ };
	feedbackReq = 0;
}

void samplesReceived(){
	ACKReceived = 0;
}

bool samplesReady(){
	return ACKReceived;
}


//*****************************************************************************
// ISR
//*****************************************************************************

__interrupt void cla1Isr6 (){
	// If CLA processed a command, it sends 'command_ack'. Afterwards, we check which
	// command it was and do whatever follows (i.e., answer back using the SCI)
	#if DEBUG_LATENCY
		StopCpuTimer1();
		ReloadCpuTimer1();
		StartCpuTimer1();
	#endif

	if(ack){
		//command = COMMAND_NULL;
		ACKReceived = 1;
	}

	//nextSignalID  = playNext();
	playNext();
    PieCtrlRegs.PIEACK.all = (PIEACK_GROUP1 | PIEACK_GROUP11);
}

