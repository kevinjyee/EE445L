// SysTick.c
// Runs on LM4F120/TM4C123
// Use SysTick at 50MHz to keep track of time. Each time
// 	SysTick interrupts, increment time by one decisecond. (1/10th a second)
// 	Keep a heartbeat using PF2 while SysTick is keeping the time.
// Stefan Bordovsky and Kevin Yee
// February 16, 2017

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
#include "Timer1.h"
#include "Timer3.h"
#include "Timer0A.h"
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value
#define SYSTICK_RELOAD	0xF42 // Reload value for an interrupt frequency of 6.4kHz.


void (*STPeriodicTask)(void);   // user function

uint32_t tempo_Counter;		// A counter used to time each beat.
uint32_t max_Tempo_Count;	// When tempo_Counter reaches this number, reset and move to next beat.
uint32_t beats_Passed = 0; 		// A counter for the number of elapsed beats.
uint32_t following_Steps; // A boolean indicating if tracking step movement to play notes.



// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void(*task)(void), uint32_t tempo){

  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
	STPeriodicTask = task;          // user function
	max_Tempo_Count = (60 * 100 * 2) / tempo;  // 60 BPM * 100 (to avoid flooring) * 2 (because SysTick runs at 64th note beat) / tempo.
	tempo_Counter = 0;
  NVIC_ST_RELOAD_R = SYSTICK_RELOAD - 1;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) | 0x40000000; // Priority 2
	beats_Passed = 0;
	following_Steps = 0;
  NVIC_ST_CTRL_R = 0x00000007; 					// ENABLE WITH CORE CLOCK AND INTERRUPTS
}

// Increment time on each 10Hz interrupt.
void SysTick_Handler(void){
	if(!tempo_Counter){

		(*STPeriodicTask)();                // execute user task
		/*
		if(beats_Passed){
		beats_Passed++;
	}
	if(beats_Passed == 32){
		SysTick_Halt();
		Timer0A_Halt();
	  Timer1A_Halt();
	  Timer3A_Halt();
		beats_Passed = 0;
	}
	*/
		// Something here
	}
	tempo_Counter = (tempo_Counter + 1) % max_Tempo_Count;
}

// Disable SysTick and Timers1 and 0.
void SysTick_Halt(void){
	NVIC_ST_CTRL_R = 0;
}

// ***************** Change_Tempo ****************
// Changes the tempo of whatever song is playing.
// Inputs:  uint8_t tempo, based on current walking speed of user.
// Outputs: none
void Change_Tempo(uint8_t tempo){
	NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
	max_Tempo_Count = (60 * 100 * 2) / tempo;  // 60 BPM * 100 (to avoid flooring) * 2 (because SysTick runs at 64th note beat) / tempo.
  NVIC_ST_RELOAD_R = SYSTICK_RELOAD - 1;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) | 0x40000000; // Priority 2
  NVIC_ST_CTRL_R = 0x00000007; 					// ENABLE WITH CORE CLOCK AND INTERRUPTS
}

// ***************** Orchestrate_Steps ****************
// Plays a quarter note worth of music after a step.
// Inputs:  None
// Outputs: None
void Orchestrate_Steps(){
	SysTick_Halt();
	Timer0A_Halt();
	Timer1A_Halt();
	Timer3A_Halt();
	beats_Passed = 1;
	NVIC_ST_CTRL_R = 0x00000007; 					// ENABLE WITH CORE CLOCK AND INTERRUPTS
}
