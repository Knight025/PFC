/*
 * CLA_Command_Int.h
 *
 *  Created on: 11/1/2017
 *      Author: Santiago Real
 */

#ifndef CLA_COMMAND_INT_H_
#define CLA_COMMAND_INT_H_

#include "Common_Defines.h"



void CLA_readOutputBuff(uint16_t*);
void CLA_writeInputBuff(uint16_t*);
void CLA_clearInputBuff(void);
void CLA_requestFeedback(void);
void CLA_waitACK(void);
void CLA_startStimulation(void);
void CLA_stopStimulation(void);

void samplesReceived();
bool samplesReady();

uint16_t playNext(void); //returns 0 if everything went smoothly


// Useful Timer Operations (from F2837xS_Examples.h):
//
// Start Timer:
#define StartCpuTimer0()   CpuTimer0Regs.TCR.bit.TSS = 0

// Stop Timer:
#define StopCpuTimer0()   CpuTimer0Regs.TCR.bit.TSS = 1

// Reload Timer With period Value:
#define ReloadCpuTimer0() CpuTimer0Regs.TCR.bit.TRB = 1

// Read 32-Bit Timer Value:
#define ReadCpuTimer0Counter() CpuTimer0Regs.TIM.all

// Read 32-Bit Period Value:
#define ReadCpuTimer0Period() CpuTimer0Regs.PRD.all


#endif /* INCLUDE_CLA_COMMAND_INT_H_ */
