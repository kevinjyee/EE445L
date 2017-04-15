// tach.c
// Runs on LM4F120/TM4C123
// Use Timer0A in 24-bit edge time mode to request interrupts on the rising
// edge of PB6 (T0CCP0), and measure period between pulses.
// Daniel Valvano
// May 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 7.2, Program 7.2

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

// external signal connected to PB6 (T0CCP0) (trigger on rising edge)
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"

#define NVIC_EN0_INT19          0x00080000  // Interrupt 19 enable
#define PF2                     (*((volatile uint32_t *)0x40025010))
#define TIMER_TAMR_TACMR        0x00000004  // GPTM TimerA Capture Mode
#define TIMER_TAMR_TAMR_CAP     0x00000003  // Capture mode
#define TIMER_CTL_TAEN          0x00000001  // GPTM TimerA Enable
#define TIMER_CTL_TAEVENT_POS   0x00000000  // Positive edge
#define TIMER_CTL_TAEVENT_NEG   0x00000004  // Negative edge
#define TIMER_CTL_TAEVENT_BOTH  0x0000000C  // Both edges
#define TIMER_IMR_CAEIM         0x00000004  // GPTM CaptureA Event Interrupt
                                            // Mask
#define TIMER_ICR_CAECINT       0x00000004  // GPTM CaptureA Event Interrupt
                                            // Clear
#define TIMER_TAILR_TAILRL_M    0x0000FFFF  // GPTM TimerA Interval Load
                                            // Register Low
#define NVIC_EN0_INT20          0x00100000  // Interrupt 20 enable

#define CONVERSION_CONSTANT 2000

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint32_t Period;              // (1/clock) units
uint32_t First;               // Timer0A first edge
int32_t Done;                 // set each rising
// max period is (2^24-1)*12.5ns = 209.7151ms
// min period determined by time to run ISR, which is about 1us
void Tach_Init(void){
	volatile uint32_t delay; 
	DisableInterrupts();
  SYSCTL_RCGCTIMER_R |= 0x01;// activate timer0    
	 delay = SYSCTL_RCGCTIMER_R;   
  SYSCTL_RCGCGPIO_R |= 0x22;       // activate port B and port F
                                   // allow time to finish activating
  First = 0;                       // first will be wrong
  Done = 0;                        // set on subsequent
  GPIO_PORTB_DIR_R &= ~0x80;       // make PB7 in
  GPIO_PORTB_AFSEL_R |= 0x80;      // enable alt funct on PB7/T0CCP1
  GPIO_PORTB_DEN_R |= 0x80;        // enable digital I/O on PB7
                                   // configure PB7 as T0CCP1
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x0FFFFFFF)+0x70000000;
  GPIO_PORTB_AMSEL_R &= ~0x80;     // disable analog functionality on PB6
  GPIO_PORTF_DIR_R |= 0x04;        // make PF2 out (PF2 built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;     // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;        // enable digital I/O on PF2
                                   // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PF
 

	TIMER0_CTL_R &= ~TIMER_CTL_TBEN; // disable timer0B during setup
  //TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER; // configure for 16-bit timer mode
                                   // configure for 32-bit capture mode
	 TIMER0_CFG_R = TIMER_CFG_16_BIT;																 
  TIMER0_TBMR_R = (TIMER_TBMR_TBCMR|TIMER_TAMR_TAMR_CAP);
                                   // configure for rising edge event
  TIMER0_CTL_R &= ~(TIMER_CTL_TBEVENT_POS|0xC);
  TIMER0_TBILR_R = TIMER_TBILR_M;// max start value for TImer B 
  TIMER0_TBPR_R = 0xFF;            // activate prescale, creating 24-bit
  TIMER0_IMR_R |= TIMER_IMR_CBEIM; // enable capture match interrupt
  TIMER0_ICR_R = TIMER_ICR_CBECINT;// clear timer0B capture match flag
  TIMER0_CTL_R |= TIMER_CTL_TBEN;  // enable timer0A 16-b, +edge timing, interrupts
                                   // Timer0A=priority 2
 
 NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFFFF00)|0x00000040; // bits 5-7
  NVIC_EN0_R |= NVIC_EN0_INT20;    // enable interrupt 20 in NVIC
	EnableInterrupts();
}
void Timer0B_Handler(void){
  PF2 = PF2^0x04;  // toggle PF2
  PF2 = PF2^0x04;  // toggle PF2
  TIMER0_ICR_R = TIMER_ICR_CBECINT;// acknowledge timer0A capture match
  Period = (First - TIMER0_TBR_R)&0xFFFFFF;// 24 bits, 12.5ns resolution
  First = TIMER0_TBR_R;            // setup for next
  Done = 1;
  PF2 = PF2^0x04;  // toggle PF2
}

uint16_t Tach_Read(void)
{
	if(Done == 1)
	{
		Done = 0;
		return Period/CONVERSION_CONSTANT;
		
	}
	else{
		return 0;
	}
}
