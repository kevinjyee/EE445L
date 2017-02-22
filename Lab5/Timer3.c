/* File Name:    Timer3.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program initializes Timer3 to interrupt every ~100ms,
 * 		maintaining a tempo at which it calls PWM_Sine to change notes played.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 2/08/2017	
 */

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
  Program 7.5, example 7.6

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
#include "Timer3.h"
#include "PWMSine.h"
#include "FSM.h"

#define RELOAD_100Hz	0x7A120 // Reload value for an interrupt frequency of 10Hz.

uint32_t tempo_Max_Count;
uint32_t tempo_Counter;


// ***************** Disable_Timer3 ****************
// Disables Timer3 to stop PMWSine calls.
// Inputs:  none
// Outputs: none
void Disable_Timer3(void){
	TIMER3_CTL_R = 0x00000000;
}

// ***************** Timer3_Init ****************
// Activate Timer3 interrupts periodically
// Inputs:  period in units (1/clockfreq)
// Outputs: none
void Timer3_Init(unsigned long tempo){
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = RELOAD_100Hz - 1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear TIMER3A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x40000000; // 8) priority 2
// interrupts enabled in the main program after all devices initialized
// vector number 51, interrupt number 35
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
	
	tempo_Max_Count = (60 * 100) / (tempo * 4); // Timer3A counts every 100ms--tempo_Max_Count is number of 100ms to make one beat.
	tempo_Counter = 0;
}

// Every time Timer3A interrupts, check for a change in song selection.
void Timer3A_Handler(void){
	if(tempo_Counter == 0){ // Every time tempo_Counter wraps around (one full beat), trigger PWM_Sine and play a new note.
		
	}
	tempo_Counter = (tempo_Counter + 1) % tempo_Max_Count; // Increment tempo_Counter and wrap if hits tempo_Max_Count.
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
}
