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

uint16_t samples[SAMPLE_SIZE];
volatile uint32_t numSamples = 0;

void sample_ADC(){
	uint16_t data;
	data = ADC0_InSeq3();
	numSamples++;
	if(numSamples == 100){
		Timer1A_Halt();
	}
}

void clear_Samples(){
	for(int i = 0; i < SAMPLE_SIZE; i++){
		samples[i] = 0;
	}
}

int main(void){ int32_t data;
  PLL_Init(Bus80MHz);   // 80 MHz
  UART_Init();              // initialize UART device
  ADC0_InitSWTriggerSeq3_Ch9();
	clear_Samples();
	Timer1_Init(&sample_ADC, FS_RELOAD);
  while(numSamples < 100){
  }
	for(int i = 0; i < SAMPLE_SIZE; i++){
		UART_OutString("\n\rADC data =");
    UART_OutUDec(samples[i]);
	}
}


