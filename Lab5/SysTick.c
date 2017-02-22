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
extern volatile uint32_t AlarmTime;

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
	volatile unsigned long delay;
	
	SYSCTL_RCGCGPIO_R |= 0x00000020; // activate port F
	int x=0;
   x++;
   x--;
   x++;	//    allow time for clock to stabilize
  GPIO_PORTF_DIR_R |= 0x0E;   // make PF123 output (PF1 built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x0E;// disable alt funct on PF123
  GPIO_PORTF_DEN_R |= 0x0E;   // enable digital I/O on PF123
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFFF0F)+0x00000000; // no alternate functions 
  GPIO_PORTF_AMSEL_R &= ~0x0E;     // disable analog functionality on PF123
}

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(uint32_t period){
	PortF_Init();
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = period - 1;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) | 0x40000000; // Priority 2
  NVIC_ST_CTRL_R = 0x00000007; 					// ENABLE WITH CORE CLOCK AND INTERRUPTS
}

// Increment time on each 10Hz interrupt.
void SysTick_Handler(void){
	PF2 ^= 0x04; // Heartbeat while SysTick is counting (so, like, always)
	PF2 ^= 0x04;
	// Something here
	PF2 ^= 0x04;
}
