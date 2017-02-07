// SysTick.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize the SysTick module, wait at least a
// designated number of clock cycles, and wait approximately a multiple
// of 10 milliseconds using busy wait.  After a power-on-reset, the
// LM4F120 gets its clock from the 16 MHz precision internal oscillator,
// which can vary by +/- 1% at room temperature and +/- 3% across all
// temperature ranges.  If you are using this module, you may need more
// precise timing, so it is assumed that you are using the PLL to set
// the system clock to 50 MHz.  This matters for the function
// SysTick_Wait10ms(), which will wait longer than 10 ms if the clock is
// slower.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the books
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
   
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 2.11, Section 2.6

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

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value
#define SYSTICK_RELOAD	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.

volatile uint8_t dSeconds; // deciSeconds (one tenth of a second)
volatile uint8_t Seconds;
volatile uint8_t Minutes;
volatile uint8_t Hours;
volatile uint8_t Meridian;

extern volatile uint32_t Time;

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(uint32_t period){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = period - 1;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) | 0x40000000; // Priority 2
  NVIC_ST_CTRL_R = 0x00000007; 					// ENABLE WITH CORE CLOCK AND INTERRUPTS
	dSeconds = 0;
	Seconds = 0;
	Minutes = 0;
	Hours = 12; // Initialize to 12:00AM
	Meridian = 0;
	Time = (1000000 * 12);
}

// Encodes time in an integer as (Meridian)hh0mm0ss.
// Keep count in deciseconds so SysTick reload value is 24-bits.
void incrementTime(){
	dSeconds++;
	if(dSeconds == 10){
		dSeconds = 0;
		Seconds++;
		if(Seconds == 60){
			Seconds = 0;
			Minutes++;
			if(Minutes == 60){
				Minutes = 0;
				Hours++;
				if(Hours == 13){
					Hours = 1;
				} else if(Hours == 12){
					Meridian ^= 1;
				}
			}
		}
	Time = (100000000 * Meridian) + (1000000 * Hours) + (1000 * Minutes) + Seconds;
	}
}

void SysTick_Handler(void){
	incrementTime();
}

// Sets current time of alarm clock.
void SysTick_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t meridian){
	dSeconds = 0;
	Seconds = seconds;
	Minutes = minutes;
	Hours = hours;
	Meridian = meridian;
	NVIC_ST_RELOAD_R = SYSTICK_RELOAD - 1; // TODO: Is this dangerous to do?
}

/*
// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 20 nsec for 50 MHz clock)
void SysTick_Wait(uint32_t delay){
  volatile uint32_t elapsedTime;
  uint32_t startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}
// Time delay using busy wait.
// This assumes 50 MHz system clock.
void SysTick_Wait10ms(uint32_t delay){
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(500000);  // wait 10ms (assumes 50 MHz clock)
  }
}
*/
