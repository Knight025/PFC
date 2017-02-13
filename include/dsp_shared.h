#ifndef _DSP_SHARED_H_
#define _DSP_SHARED_H_
//#############################################################################
//
//#############################################################################

//*****************************************************************************
// includes
//*****************************************************************************
#include "F28x_Project.h"
#include "F2837xS_Cla_defines.h"
#include <stdint.h>
#include "Common_Defines.h"

//*****************************************************************************
// defines
//*****************************************************************************

// Nota: las senales se dividen en rafagas con pulsos distintos; asi que guardo
// cada rafaga en un array de longitud SIGNALS_BUFFER_LENGTH, y cada pulso debe
// tener como maximo SIGNAL_LENGTH muestras.
//#define SIGNALS_BUFFER_LENGTH		1024	// Tamano del buffer de rafaga de senales
//#define SIGNAL_LENGTH				100		// Longitud maxima de cada pulso en muestras (NOTA: DEBE SER > 64)
//#define N_SIGNALS_BUFFER			5		// Numero maximo de rafagas de senales para almacenar
//#define DAC_ZERO					2048	// Bipolar signal, DAC 12 bits -> (2^12/2) = 2^11 = 2048

//**********************************************************************************************************AQUÍ SERÍA MEJOR USAR UN ENUMERADO!!



//DON'T CHANGE IT. I need to split both signal transfer buffers: CLA->C28, C28->CLA
//Page length = 64, 2 pages. Be careful so there's enough room for the other variables
#define SIGNAL_LENGTH_ABUFF 64
#define SIGNAL_LENGTH_BBUFF (SIGNAL_LENGTH - 64)
//*****************************************************************************
// typedefs
//*****************************************************************************
struct SIGNALDATA {                  // bits description
	uint16_t nSignals;
	uint16_t signalsLength;
    uint16_t signals[SIGNAL_LENGTH];
};





//*****************************************************************************
// globals
//*****************************************************************************
// Note (Santi): it seems that CLA does not support globals, so I have to initialize them
// at the C28 as a shared variable (later I give control over those memory blocks to the
// CLA (LS0, LS1, ...):

// Communication variables
//CPU->CLA
//extern uint16_t signalID;			//Contains signal ID
extern uint16_t feedbackReq;
extern uint16_t signalBuff[];
extern uint16_t signalBuff2[];

//CLA->CPU
extern uint16_t ack;
extern uint16_t volt[];					//Result (output)
extern uint16_t volt2[];




// Internal task variables

//Task 6 (C) Variables
extern uint16_t DACsamples[];
extern uint16_t signal[];
//extern uint16_t nextSignalID;

//Task 7 (C) Variables
extern uint16_t* pSignal;
extern uint16_t* pDACsamples;
extern uint16_t endSignalCounter;

//Task 8 (C) Variables

//Common (C) Variables
//extern float PIBYTWO;
//extern float CLAatan2Table[];




// (Santi) CLA to C28x Interrupt Functions (executed in C28x)
void CLA_configClaMemory();
void CLA_initCpu1Cla1();
void CLA_init_variables();
__interrupt void cla1Isr1 ();
__interrupt void cla1Isr2 ();
__interrupt void cla1Isr3 ();
__interrupt void cla1Isr4 ();
__interrupt void cla1Isr5 ();
__interrupt void cla1Isr6 ();
__interrupt void cla1Isr7 ();
__interrupt void cla1Isr8 ();

//*****************************************************************************
// function prototypes
//*****************************************************************************
// The following are symbols defined in the CLA assembly code
// Including them in the shared header file makes them
// .global and the main CPU can make use of them.

//CLA C Tasks
__interrupt void Cla1Task1();
__interrupt void Cla1Task2();
__interrupt void Cla1Task3();
__interrupt void Cla1Task4();
__interrupt void Cla1Task5();
__interrupt void Cla1Task6();
__interrupt void Cla1Task7();
__interrupt void Cla1Task8();



// PWM1 configuration utils
#define clearEpwm1Counter() EPwm1Regs.TBCTR = 0x0000
#define stopEPWM1() EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE
#define startEPWM1() EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP


#endif //PRUEBA_SHARED_H_ definition
