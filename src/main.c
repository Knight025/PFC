#include "dsp_shared.h"
#include "peripheralsConfig.h"
#include "serial.h"
#include "interp.h"
#include "playIt.h"
//#include "CLA_Command_Int.h"


//*****************************************************************************
// Defines
//*****************************************************************************
#define BAUD_RATE			115200	// Max speed allowed with Matlab: 921600
									// (If signals length = 100 -> 100*MAX_ADC_SIGNALS samples)
//Baud Rate: 95.3659 - 6250000


//*****************************************************************************
// Common variables
//*****************************************************************************

//bool answerReady;
//#pragma DATA_SECTION(answerDataBuff, "ramgs2");
//uint16_t answerDataBuff[SIGNAL_LENGTH*MAX_ADC_SIGNALS];

//*****************************************************************************
// Function prototypes
//*****************************************************************************
int playListRequest(uint16_t*);
int stopStimulation(uint16_t*);
int restartStimulation(uint16_t*);
int signalRequest(uint16_t*);
int ADCSamplesRequest(uint16_t*);
int signalReceived(uint16_t*);
int newPlayList(uint16_t*);

__interrupt void cla1Isr6();



void main(void){

	InitSysCtrl();


	// Step 1. Initialize GPIO:
	// This example function is found in the F2837xS_Gpio.c file and
	// illustrates how to set the GPIO to it's default state.
	InitGpio();
	// For this example, only init the pins for the SCI-A port.
	//  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
	//  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
	// These functions are found in the F2837xS_Gpio.c file.
	GPIO_SetupPinMux(85, GPIO_MUX_CPU1, 5);
	GPIO_SetupPinOptions(85, GPIO_INPUT, GPIO_ASYNC);
	GPIO_SetupPinMux(84, GPIO_MUX_CPU1, 5);
	GPIO_SetupPinOptions(84, GPIO_OUTPUT, GPIO_PUSHPULL);



	// Step 2. Enable PWM1 and its GPIOs
	CpuSysRegs.PCLKCR2.bit.EPWM1=1;

	// (Santi) Following code lines aren't needed
	//Init GPIO pins for ePWM1
	//These functions are in the F2837xS_EPwm.c file
	//InitEPwm1Gpio();
	//LED_configure();

	// (Santi) InitPieCtrl() includes DINT, so I removed this code line
	//DINT;


	InitPieCtrl();

	IER = 0x0000;
	IFR = 0x0000;
	
	InitPieVectTable();

	// Step 3. Init timers
	init_timers();

	// Step 4. Configure the CLA memory spaces first followed by CLA's task vectors.
	CLA_init_variables();
	CLA_configClaMemory();
	CLA_initCpu1Cla1();


	// Step 5. Configure the ADC to start sampling, triggered by EPWM1 interruption
	ADC_initAdcA();

	// Step 6. (Santi) DAC Configuration
	initDAC();

	// Step 7. Configure EPWM1 to trigger ADC
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	EPWM_initEpwm();


	// Step 8. Enable global Interrupts and higher priority real-time debug events:
	EINT;	// Enable Global interrupt INTM
	ERTM;	// (Santi) Should I remove this line in FLASH code? (after debugging)


	// Step 9. Turn on the EPWM1
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; //unfreeze, and enter up count mode
	EDIS;


	// Step 10. Init SCI
	scia_init(BAUD_RATE);


	// Step 11. Add commands
	interp_addcmd(newPlayList, "newPlayList: sends a new 'stimulation signal playList', event-based");
	interp_addcmd(stopStimulation, "stopStimulation: stops stimulation play list");
	interp_addcmd(restartStimulation, "restartStimulation: restarts stimulation play list");
	interp_addcmd(signalRequest, "signalRequest: asks for 'ID' signal from CLA's RAM");
	//interp_addcmd(ADCSamplesRequest, "ADCSamplesRequest: Sends last ADC samples buff (NOT IMPLEMENTED YET)");
	interp_addcmd(signalReceived, "sendSignal: sends a stimulation signal to be saved on CLA's RAM, with an ID associated");

	while(1){
		while(receive_pkg() == 0) { };
		execute_pkg(getPkg());
	}
}





//*****************************************************************************
// SCI commands function definitions
//*****************************************************************************

int stopStimulation(uint16_t* arg){
	playListStop();
	return 1;
}

int restartStimulation(uint16_t* arg){
	playListRestart();
	return 1;
}

int signalRequest(uint16_t* arg) {
	uint16_t id = *arg;
	uint16_t ansData[SIGNAL_LENGTH];

	getSignal_playList(id, ansData);

	scia_pkg(ansData, SIGNAL_LENGTH);

	return 1;
}

int ADCSamplesRequest(uint16_t* arg){
	/*
	NOT SOPORTED
	*/
	return 1;
}

int signalReceived(uint16_t* arg){
	uint16_t id = *arg++;
	saveSignal_playList(id, arg);

	return 1;
}

int newPlayList(uint16_t* arg){
	playList_pkg playList;
	uint16_t answerLength, *pAnswer;

	//Extract pkg data
	playList.length = *arg++;
	playList.options.all = *arg++;
	playList.list = (next_Signal*) arg;

	loadPlayList(playList);

	if (playList.options.data.feedback){
		answerLength = 0;
		playListRestart();
		waitForFeedback();
		pAnswer = getFeedback();
		answerLength = getFeedbackLength();
		scia_pkg(pAnswer, answerLength*SIGNAL_LENGTH);
	}

	return 1;
}
