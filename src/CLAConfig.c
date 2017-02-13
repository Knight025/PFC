/*
 * CLADefault.c
 *
 *  Created on: 20/6/2016
 *      Author: Santiago Real (based on TI's firmware code)
 */
#include "dsp_shared.h"


//*****************************************************************************
// CLA Code
//*****************************************************************************

//CLA defines
//#define WAITSTEP 	asm(" RPT #255 || NOP")

//Common

// Globals
// NOTE: Do not initialize the Message RAM variables globally, they will be reset
// during the message ram initialization phase in the CLA memory configuration routine

#pragma DATA_SECTION(volt,"Cla1ToCpuMsgRAM");
uint16_t volt[SIGNAL_LENGTH_ABUFF];
#pragma DATA_SECTION(volt2, "Cla1ToCpuMsgRAM");
uint16_t volt2[SIGNAL_LENGTH_BBUFF];
#pragma DATA_SECTION(ack, "Cla1ToCpuMsgRAM");
uint16_t ack;
//#pragma DATA_SECTION(signalID, "CpuToCla1MsgRAM");
//uint16_t signalID;
#pragma DATA_SECTION(feedbackReq, "CpuToCla1MsgRAM");
uint16_t feedbackReq;
#pragma DATA_SECTION(signalBuff, "CpuToCla1MsgRAM");
uint16_t signalBuff[SIGNAL_LENGTH_ABUFF];
#pragma DATA_SECTION(signalBuff2, "CpuToCla1MsgRAM");
uint16_t signalBuff2[SIGNAL_LENGTH_BBUFF];
#pragma DATA_SECTION(pSignal, "CLADataLS0");
uint16_t* pSignal;
#pragma DATA_SECTION(endSignalCounter, "CLADataLS0");
uint16_t endSignalCounter;
//#pragma DATA_SECTION(nextSignalID, "CLADataLS0");
//uint16_t nextSignalID;
#pragma DATA_SECTION(pDACsamples, "CLADataLS0");
uint16_t* pDACsamples;


//#ifdef __cpluspluspulseFrequency
//#pragma DATA_SECTION("CLADataLS1")
//#else
#pragma DATA_SECTION(signal,"CLADataLS1")
//#endif //__cplusplus
uint16_t signal[SIGNAL_LENGTH];


#pragma DATA_SECTION(DACsamples, "CLADataLS0")
uint16_t DACsamples[SIGNAL_LENGTH];



//*****************************************************************************
// C28x Code
//*****************************************************************************


void CLA_configClaMemory(void);
void CLA_initCpu1Cla1(void);
void init_standard_signals(void);



void CLA_configClaMemory(void){
	extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;

	EALLOW;

#ifdef _FLASH
	// Copy over code from FLASH to RAM
	memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart,
			(uint32_t)&Cla1funcsLoadSize);
#endif //_FLASH

	// Initialize and wait for CLA1ToCPUMsgRAM
	MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
	while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1){};

	// Initialize and wait for CPUToCLA1MsgRAM
	MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
	while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1){};

	// Select LS5RAM to be the programming space for the CLA
	// First configure the CLA to be the master for LS5 and then
	// set the space to be a program block
	MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1;


	// Next configure LS0RAM as data spaces for the CLA
	// First configure the CLA to be the master for LS0(1) and then
    // set the spaces to be code blocks
	MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 0;

	// Next configure LS1RAM as data spaces for the CLA
	// First configure the CLA to be the master for LS1(1) and then
    // set the spaces to be code blocks
	MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1;
	MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 0;

	EDIS;
}

void CLA_initCpu1Cla1(void)
{
	// Compute all CLA task vectors
	// On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
	// opposed to offsets used on older Type-0 CLAs
	EALLOW;
	Cla1Regs.MVECT1 = (uint16_t)(&Cla1Task1);
	Cla1Regs.MVECT2 = (uint16_t)(&Cla1Task2);
	Cla1Regs.MVECT3 = (uint16_t)(&Cla1Task3);
	Cla1Regs.MVECT4 = (uint16_t)(&Cla1Task4);
	Cla1Regs.MVECT5 = (uint16_t)(&Cla1Task5);
	Cla1Regs.MVECT6 = (uint16_t)(&Cla1Task6);
	Cla1Regs.MVECT7 = (uint16_t)(&Cla1Task7);
	Cla1Regs.MVECT8 = (uint16_t)(&Cla1Task8);

	// Enable the IACK instruction to start a task on CLA in software
	// for all  8 CLA tasks. Also, globally enable all 8 tasks (or a
	// subset of tasks) by writing to their respective bits in the
	// MIER register
	Cla1Regs.MCTL.bit.IACKE = 1;
    Cla1Regs.MIER.all 	= (M_INT8 | M_INT7 | M_INT6); //(Santi) Originalmente: (M_INT8 | M_INT7)

    // Configure the vectors for the end-of-task interrupt for CLA Task
    // (Santi) I changed it to use the default ISR at 'F2837xS_DefaultISR.c'
    // i.e., CLA1_1_ISR
	PieVectTable.CLA1_6_INT   = &cla1Isr6;


	// Set the adca.1 as the trigger for task 7
	DmaClaSrcSelRegs.CLA1TASKSRCSEL2.bit.TASK7 = CLA_TRIG_ADCAINT1;
	// Set the timer0 as the trigger for task 6
	DmaClaSrcSelRegs.CLA1TASKSRCSEL2.bit.TASK6 = CLA_TRIG_TINT0;
	// Software trigger for task 5
	//DmaClaSrcSelRegs.CLA1TASKSRCSEL2.bit.TASK5 = CLA_TRIG_NOPERPH;

	// (Santi) Enable interrupts
	//PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
	//PieCtrlRegs.PIEIER11.all  = 0xFFFF; // Enable CLA interrupts at the group and subgroup levels

	PieCtrlRegs.PIEIER11.bit.INTx6 = 1;  //Habilito solo CLA Task6
	IER |= (M_INT11);
	EDIS;
}

void CLA_init_variables(void){
	pSignal = 0;
	endSignalCounter = SIGNAL_LENGTH;
	//nextSignalID = 0;
	//command = COMMAND_NULL;
	feedbackReq = 0;
	int i = 0;

	for (i = 0; i < SIGNAL_LENGTH_ABUFF; i++){
		signalBuff[i] = DAC_ZERO;
	}
	for (i = 0; i < SIGNAL_LENGTH_BBUFF; i++){
		signalBuff2[i] = DAC_ZERO;
	}

	while(i < SIGNAL_LENGTH){
		signal[i] = 0x0000;
		i++;
	}
	signalBuff[38] = DAC_ZERO;
	signalBuff[39] = DAC_ZERO;
	signalBuff[40] = DAC_ZERO;
	signalBuff[41] = DAC_ZERO;
}




