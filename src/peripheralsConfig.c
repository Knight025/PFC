/*
 * peripherals.c
 *
 *  Created on: 20/6/2016
 *      Author: Santiago Real (based on TI's firmware example code)
 */

#include "peripheralsConfig.h"
#include "F28x_Project.h"
#include <stdint.h>
#include "Common_Defines.h"
//#include "F2837xS_cputimervars.h"


//*****************************************************************************
// defines
//*****************************************************************************
//ADC defines
//definitions for selecting ADC reference
//CAUTION: internal reference mode will drive a voltage onto the VREF pin,
//make sure that this pin is not also being driven externally in this case
#define REFERENCE_INTERNAL 0 //internal reference (12-bit only)
#define REFERENCE_EXTERNAL 1 //external reference

//EPWM defines
//(Santi) Period values range for 16 bits: 64000 aprox
// PWM1_PERIOD = 1000 -> frec. = 100KHz
// PWM2_PERIOD = 250   -> frec. = 3.125KHz
#define PWM1_PERIOD		    100*ADC_PERIOD //2000
#define PWM1_DUTY_CYCLE		50  //1000
#define PWM2_PERIOD		    78	//1000
#define PWM2_DUTY_CYCLE		39

//DAC defines
#define DACB				2


//initialize DAC pointer
volatile struct DAC_REGS* DAC_PTR[4] = {0x0,&DacaRegs,&DacbRegs,&DaccRegs};


void EPWM_initEpwm(void);
void InitEPwm1(void);
void InitEPwm2(void);
void ADC_initAdcA(void);
void configureDAC(Uint16 dac_num);



void init_timers(void){
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
	/*
	EALLOW;  // This is needed to write to EALLOW protected registers
	PieVectTable.TIMER0_INT = &cpu_timer0_isr;
	PieVectTable.TIMER1_INT = &cpu_timer1_isr;
	PieVectTable.TIMER2_INT = &cpu_timer2_isr;
	EDIS;    // This is needed to disable write to EALLOW protected registers
	*/

    // Step 4. Initialize the Device Peripheral. This function can be
    //         found in F2837xS_CpuTimers.c
	InitCpuTimers();   // For this example, only initialize the Cpu Timers

    // Configure CPU-Timer 0, 1, and 2 to interrupt every second:
    // 200MHz CPU Freq, 1 second Period (in uSeconds)
    ConfigCpuTimer(&CpuTimer0, 200, 2000);
	#if DEBUG_LATENCY
    ConfigCpuTimer(&CpuTimer1, 200, 1000000);
	#endif
    //ConfigCpuTimer(&CpuTimer2, 200, 1000000);

    // To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
    // of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in F2837xS_cputimervars.h), the
    // below settings must also be updated.
    //CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    //CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0;
    //CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

}


void EPWM_initEpwm(void){
	InitEPwm1();
	//InitEPwm2();
}

void initDAC(){
	configureDAC(DACB);
}

void InitEPwm1(){
	EALLOW;
	//NOTA: hay que deshabilitar el contador mientras se configura; varias opciones:
	//EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 		// freeze counter
	//CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;			// Turn off the EPWM clock
	//Por ahora he optado por deshabilitar el reloj del modulo PWM fuera de esta funcion

	EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; 		// (Santi) cambio de 3 (freeze counter) a 0 (count up)
	EPwm1Regs.TBPRD = PWM1_PERIOD;			   		// Set period to PWM1_PERIOD counts
	//EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;  		// Disable phase loading
	//EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;       	// Phase is 0
	//EPwm1Regs.TBCTR = 0x0000;               		// Clear counter
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
	//EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV2;

	// Setup shadow register load on ZERO
	//EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	//EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	//EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	//EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;



	// Set Compare values
	EPwm1Regs.CMPA.bit.CMPA = PWM1_DUTY_CYCLE;  	// Set compare A value to PWM1_DUTY_CYCLE counts
	//EPwm1Regs.CMPB.bit.CMPB = EPWM1_MIN_CMPB;    // Set Compare B value

	// Set action
	//EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM1A on Zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;          	// Clear PWM1A on Zero
    EPwm1Regs.AQCTLA.bit.PRD = AQ_CLEAR;
    //EPwm1Regs.AQCTLB.bit.ZRO = AQ_SET;            // Set PWM1B on Zero
    //EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Clear PWM1B on event B, up count


    // Interrupt where we will change the Compare Values
    EPwm1Regs.ETSEL.bit.SOCAEN = 1; 				//enable SOCA
	EPwm1Regs.ETSEL.bit.SOCASEL	= 4;	        	// Select SOC on up-count
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;		        	// Generate pulse on 1st event
	//EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
	//EPwm1Regs.ETSEL.bit.INTEN = 1;                // Enable INT
	//EPwm1Regs.ETPS.bit.INTPRD = ET_3RD;           // Generate INT on 3rd event

	EDIS;
}

void InitEPwm2(){
	EALLOW;
	//CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;			// Turn off the EPWM clock

	EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  	// Count up mode
	EPwm2Regs.TBPRD = PWM2_PERIOD;			    		// Set period to PWM2_PERIOD counts
	//EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;    	// Disable phase loading
	//EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;       	// Phase is 0
	//EPwm2Regs.TBCTR = 0x0000;                  	// Clear counter
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
	EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV128; 		// (Clk/128)

	// Setup shadow register load on ZERO
	//EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
	//EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	//EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	//EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

	// Set Compare values
	EPwm2Regs.CMPA.bit.CMPA = PWM2_DUTY_CYCLE;  	// Set compare A value to PWM2_DUTY_CYCLE counts
	//EPwm1Regs.CMPB.bit.CMPB = EPWM1_MIN_CMPB;    	// Set Compare B value

	// Set action
	//EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM2A on Zero
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;          	// Clear PWM2A on Zero
    EPwm2Regs.AQCTLA.bit.PRD = AQ_CLEAR;
    //EPwm2Regs.AQCTLB.bit.ZRO = AQ_SET;            // Set PWM2B on Zero
    //EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Clear PWM1B on event B, up count

    // Interrupt configuration (Santi: this seccion original purpose was to 'change the Compare Values')
    //EPwm2Regs.ETSEL.bit.SOCAEN = 1; 				//enable SOCA
	//EPwm2Regs.ETSEL.bit.SOCASEL = 4;	        	// Select SOC on up-count
	//EPwm2Regs.ETPS.bit.SOCAPRD = 1;		        // Generate pulse on 1st event
	EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;    	// Select INT on -- (beware, review "ETSEL define". 5? 4? 1?)
	EPwm2Regs.ETSEL.bit.INTEN = 1;            		// Enable INT
	EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;       		// Generate INT on 1st event

    EDIS;
}



void ADC_initAdcA(void){
//Configure the ADC and power it up
	uint16_t i;

	EALLOW;
	//write configurations
	//AdcaRegs.ADCCTL2.bit.PRESCALE = 6; 		//set ADCCLK divider  (default: 6)
	AdcaRegs.ADCCTL2.bit.PRESCALE = 1; 		//set ADCCLK divider  (default: 6)
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

	//Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

	//power up the ADC
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;

	//delay for > 1ms to allow ADC time to power up
	for(i = 0; i < 1000; i++){
		asm("   RPT#255 || NOP");
	}

//Select the channels to convert and end of conversion flag ADCA
	//Default (kept as an example)
    //AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  	//SOC0 will convert pin A0
    //AdcaRegs.ADCSOC0CTL.bit.ACQPS = 99; 	//sample window is 100 SYSCLK cycles
    //AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5; 	//trigger on ePWM1 SOCA/C
    //AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; 	//end of SOC0 will set INT1 flag
    //AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   	//enable INT1 flag
    //AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; 	//make sure INT1 flag is cleared

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  	//SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19; 	//sample window is 20 SYSCLK cycles (minimum 75 ns)
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5; 	//trigger on ePWM1 SOCA/C
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; 	//end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   	//enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; 	//make sure INT1 flag is cleared


	EDIS;
}


void configureDAC(Uint16 dac_num){
	EALLOW;
	DAC_PTR[dac_num]->DACCTL.bit.DACREFSEL = 1; 		//VREFHI = 1; REFERENCE_VDAC = 0;
	DAC_PTR[dac_num]->DACOUTEN.bit.DACOUTEN = 1; 		//DAC Output enabled
	DAC_PTR[dac_num]->DACVALS.all = 0x00AA;
	DELAY_US(10); // Delay for buffered DAC to power up
	EDIS;
}


void LED_configure(void){
	GPIO_SetupPinMux(13, GPIO_MUX_CPU1, 0);
	GPIO_SetupPinOptions(13, GPIO_OUTPUT, GPIO_PUSHPULL);
}

void LED_on(void){
	GPIO_WritePin(13, 1);
}

void LED_off(void){
	GPIO_WritePin(13, 0);
}

