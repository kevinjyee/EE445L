// Timer0A.c
// Runs on LM4F120/TM4C123
// Use Timer0A in periodic mode to request interrupts at a customizable
// period. Called by PWMSine.c to output a sine wave to produce music.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.8

  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Volume 2, Program 7.5, example 7.6

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
#include "..//inc//tm4c123gh6pm.h"
#include "Timer0A.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t StartCritical (void);// previous I bit, disable interrupts
void EndCritical(uint32_t sr);// restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*T0PeriodicTask)(void);   // user function


void No_Op0(){
}

// ***************** Timer0A_Init ****************
// Activate TIMER0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq), 32 bits
// Outputs: none
void Timer0A_Init(void(*task)(void), uint32_t period){uint32_t sr;
  sr = StartCritical();
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
	if(period){
		T0PeriodicTask = task;          // user function
	} else{
		T0PeriodicTask = &No_Op0;
	}
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
	if(period == 0){
		period = 10000;
	}
	TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
	NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
	TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x20000000; // 8) priority 1 TODO: Is this the correct priority?
	TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A unless period = 0;
  EndCritical(sr);
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
  //uint32_t sr = StartCritical();
  (*T0PeriodicTask)();                // execute user task
	//EndCritical(sr);
}

void Timer0A_Halt(void){
	TIMER0_CTL_R = 0x00000000; // Disable TIMER0A.
}
