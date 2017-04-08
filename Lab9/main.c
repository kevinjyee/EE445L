// main.c
// Runs on LM4F120/TM4C123
// UART runs at 115,200 baud rate 
// Daniel Valvano
// May 3, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
 
 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#define FS_RELOAD 80000 // Reload value for 1000Hz interrupt w/ 80MHz BUS clock.
#define SAMPLE_SIZE 100

#include <stdint.h> // C99 variable types
#include "ADCSWTrigger.h"
#include "uart.h"
#include "PLL.h"
#include "Timer1.h"
#include "ST7735.h"
#include "fixed.h"
#include "LCD.h"
/*
uint16_t ADCdatasample[53]={0,23,80,138,197,257,317,379,441,504,568,
     633,699,766,834,902,972,1043,1114,1187,1260,
     1335,1411,1487,1565,1644,1724,1805,1888,1971,2056,
     2142,2229,2317,2406,2497,2589,2682,2777,2873,2970,
     3068,3168,3270,3372,3476,3582,3689,3797,3907,4019,4095,4096};
		*/ 		 
		 //uint16_t ADCdatasample[53]={2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000,2000};
uint16_t ADCdatasample[53]={4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,};

uint16_t samples[SAMPLE_SIZE];
volatile uint32_t numSamples = 0;
volatile uint16_t ADC_MAILBOX = 0;

void ADC_Sample_Test(){
	uint16_t data;
	data = ADC0_InSeq3();
  samples[numSamples] = ADC0_InSeq3();
	numSamples++;
	if(numSamples == 100){
		Timer1A_Halt();
	}
}

void sample_ADC(){
	ADC_MAILBOX = ADC0_InSeq3();
}

void clear_Samples(){
	for(int i = 0; i < SAMPLE_SIZE; i++){
		samples[i] = 0;
	}
}

void test_ADC(){
	clear_Samples();
	Timer1_Init(&sample_ADC, FS_RELOAD);
	while(1){
		if(numSamples == 100){
			for(int i = 0; i < SAMPLE_SIZE; i++){
				UART_OutString("\n\rADC data =");
				UART_OutUDec(samples[i]);
				ST7735_printData(samples[i]);
				ST7735_plotData(samples[i]);
			}
		}
	}
}

void measure_Temperature(){
	Timer1_Init(&sample_ADC, FS_RELOAD);
}

int main(void){ int32_t data;
  PLL_Init(Bus80MHz);   // 80 MHz
  UART_Init();              // initialize UART device
  ADC0_InitSWTriggerSeq3_Ch9();
	ST7735_InitR(INITR_REDTAB);
	ST7735_XYplotInit(4096,0);
	//ADC_Sample_Test();
	measure_Temperature();
  while(1){
		ST7735_printData(ADC_MAILBOX);
		ST7735_plotData(ADC_MAILBOX);
	}

	
}


