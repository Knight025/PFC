/*
 * playIt.c
 *
 *  Created on: 29/6/2016
 *      Author: Santiago Real
 */
#include "F28x_Project.h"
#include "playIt.h"
#include "F2837xS_Cla_defines.h"
#include <stdint.h>
#include "CLA_Command_Int.h"

//Defines
//	Modificable:
#define PLAYLIST_MAX_LENGTH		2048		//Be careful as to where it is allocated
#define SIGNAL_LIB_LENGTH		40

// Reserved:
#define PAUSE_PERIOD			200000		//Time in 1/200 us until next TEMP0 interruption. For nextPulse()
#define MAX_ADC_SIGNALS			30		// Max ADC signals feedback that can be requested in one command
//#define STOP_PLAYING		2



/*************************************************
 * FOLLOWING 'DEFINE's ARE RELATED TO TIMER0 CONFIGURATION
 * It might be better to make it dependent of timer0 initialization function
 */
#define TO_MICROSEC				200			//As for now, 200 timer0 cycles = 1 us (200 MHz)

#pragma DATA_SECTION(playListBuff, "ramgs1");
next_Signal playListBuff[PLAYLIST_MAX_LENGTH] = {
		{ (uint32_t) 1<<24 | 1000 },
		{(uint32_t) 0},
};

#pragma DATA_SECTION(answerDataBuff, "ramgs2");
uint16_t answerDataBuff[SIGNAL_LENGTH*MAX_ADC_SIGNALS];

#pragma DATA_SECTION(signalsLib, "ramgs3");
uint16_t signalsLib[SIGNAL_LIB_LENGTH];



playList_pkg playList = {0, 0, playListBuff};
uint16_t playListCounter = 0;
next_Signal* lastSignalEvent = &playListBuff[1];

#if DEBUG_LATENCY
uint32_t retardo1 = 0, retardo2 = 0;
#endif


bool extraPulseSent = 0;
bool askForSamples = 0;
bool getSamples = 0;uint16_t answerLength = 0;
bool feedbackReady = 0;


void deletePlayList(void){
	playListBuff[1].data.ID = 1;
	playListBuff[1].data.time = 1000;
	playList.length = 1;
}


void loadPlayList(playList_pkg *pkg){
	int i;
	next_Signal* pList = pkg->list;

	// INCLUDE IN THIS SECTION code to prevent error due
	// to wrong data (i.e., ID > SIGNAL_LIB_LENGTH)

	StopCpuTimer0();
	playList.length = pkg->length;
	playList.options = pkg->options;
	for (i = 0; i < playList.length; i++){
		playListBuff[i] = *pList++;
	}
}


void saveSignal_playList(uint16_t id, uint16_t *data){
	int i;
	uint16_t *pLib, *pData;
	pLib = signalsLib + (id-1)*SIGNAL_LENGTH;
	pData = data;

	for (i = 0; i < SIGNAL_LENGTH; i++){
		*pLib++ = *pData++;
	}
}

void getSignal_playList(uint16_t id, uint16_t *data){
	int i;
	uint16_t *pLib, *pData;
	pLib = signalsLib + (id-1)*SIGNAL_LENGTH;
	pData = data;

	for (i = 0; i < SIGNAL_LENGTH; i++){
		*pData++ = *pLib++;
	}
}


void playListStop(void){
	StopCpuTimer0();
	//ReadCpuTimer0Period() = PAUSE_PERIOD;
	//forcePlayListStop = 1;
}


void playListRestart(void){
	next_Signal signal;

	//clear variables
	playListCounter = 0;
	answerLength = 0;
	extraPulseSent = 0;
	askForSamples = 0;
	getSamples = 0;
	feedbackReady = 0;
	CLA_clearInputBuff();

	//load first stimulation signal
	signal = playListBuff[playListCounter++];
	ReadCpuTimer0Period() = (uint32_t) TO_MICROSEC*signal.data.time;
	CLA_writeInputBuff(&signalsLib[(signal.data.ID - 1)*SIGNAL_LENGTH]);

	ReloadCpuTimer0();
	StartCpuTimer0();
}


void waitForFeedback(){
	while(!feedbackReady) { }
	feedbackReady = 0;
}

uint16_t *getFeedback(){
	return answerDataBuff;
}

uint16_t getFeedbackLength(){
	return answerLength;
}

void extraPulse() {
	CLA_clearInputBuff();
	ReadCpuTimer0Period() = PAUSE_PERIOD;
	ReloadCpuTimer0();
	StartCpuTimer0();
}


uint16_t playNext(void){
	next_Signal signal;

	if(!playList.options.data.feedback){

		if(playListCounter == playList.length){
			if(!playList.options.data.periodic){
				playListStop();
			}
			playListCounter = 0;
		}

		signal = playListBuff[playListCounter++];

		// CAMBIAR -> INCLUIR FUNCIONES EN CLA_Command_Int.c que manejen
		// el TIMER0, generando un error si no se cumplen las restricciones
		// de retardo (OJO, QUE ESTAS RESTRICCIONES VARÍAN CON EL CÓDIGO)
		ReadCpuTimer0Period() = (uint32_t) TO_MICROSEC*signal.data.time;
		CLA_writeInputBuff(&signalsLib[(signal.data.ID - 1)*SIGNAL_LENGTH]);

		#if DEBUG_LATENCY
			retardo1 = ReadCpuTimer1Counter();
		#endif

	}else{
		if(getSamples & samplesReady()){
			//GET SAMPLES
			if(answerLength < MAX_ADC_SIGNALS){
				CLA_readOutputBuff(&answerDataBuff[(answerLength++)*SIGNAL_LENGTH]);
			}else{
				playList.options.data.feedback = 0;
				feedbackReady = 1;
			}
			samplesReceived();
			getSamples = 0;
		}

		if(playListCounter == playList.length){
			if(playListBuff[playList.length-1].data.FeedbackReq & (!extraPulseSent)){
				getSamples = 1;
				CLA_requestFeedback();
				extraPulse();
				extraPulseSent = 1;
				return 0;
			}
			feedbackReady = 1;

			if(!playList.options.data.periodic){
				playListStop();
			}
			playListCounter = 0;
			playList.options.data.feedback = 0;
		}

		signal = playListBuff[playListCounter++];

		ReadCpuTimer0Period() = (uint32_t) TO_MICROSEC*signal.data.time;
		CLA_writeInputBuff(&signalsLib[(signal.data.ID - 1)*SIGNAL_LENGTH]);
		//look if last stimulating signal requested feedback
		askForSamples = playListBuff[playListCounter-2].data.FeedbackReq;

		if(askForSamples){
			CLA_requestFeedback();	//paired with CLA_waitACK()
			getSamples = 1;
		}
		#if DEBUG_LATENCY
			retardo2 = ReadCpuTimer1Counter();
		#endif
	}


	#if DEBUG_LATENCY
		StopCpuTimer1();
		ReloadCpuTimer1();
		ReadCpuTimer1Counter() = 0;
	#endif


	return 0;
}


