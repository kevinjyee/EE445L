// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
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

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdint.h>
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "fixed.h"
#include "ST7735.h"

/* Maximum value an `unsigned int' can hold.  (Minimum is 0.)  */
#define UINT_MAX  4294967295U
#define UINT_MIN 0
#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
#define NUM_SAMPLES 1000
#define MAX_ADC 4096	
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void DelayWait10s(uint32_t n);


// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.

static uint32_t ADCvalueBuffer[NUM_SAMPLES];//Debugging Dump Number 1
static uint32_t TimevalueBuffer[NUM_SAMPLES]; //Debugging Dump Number 2 

static int32_t pmfOccurences[MAX_ADC];
static int32_t ADCDefaultValues[MAX_ADC];

volatile uint32_t BufferIndex =0;

void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
	
  
	if(BufferIndex < 1000)
	{
		PF2 ^= 0x04;                   // profile
		PF2 ^= 0x04;                   // profile
		ADCvalueBuffer[BufferIndex] = ADC0_InSeq3();
		TimevalueBuffer[BufferIndex] = TIMER1_TAR_R; //12.5 ns units
		BufferIndex ++;
		  PF2 ^= 0x04;                   // profile
	}

}

/*
initializes one of the 32-bit timers to count every 12.5ns continuously
TIMER1_TAILR_R to 0xFFFFFFFF, and remove the interrupt enable and interrupt arm statements
reading the Timer 1 TIMER1_TAR_R will return the current time in 12.5ns units
measure elapsed time we read TIMER1_TAR_R twice and subtract the second measurement from the first.
12.5ns*232 is 53 seconds. So this approach will be valid for measuring elapsed times less than 53 seconds. The time measurement resolution is 12.5 ns.
*/
void Timer1_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  //PeriodicTask = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = 0xFFFFFFFF;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
 // TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  //NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

void process_Data(){
	for(int i =0; i < NUM_SAMPLES; i++)
	{
		if(ADCvalueBuffer[i] < 4096)
		{
			pmfOccurences[ADCvalueBuffer[i]] += 1;
			
		}
	}
	
}

void plot_pmf(){
	
		uint32_t maxValue = UINT_MIN;
		uint32_t minValue = UINT_MAX;
	  for(int i =0; i < NUM_SAMPLES; i++)
	{
		if(ADCvalueBuffer[i] > maxValue){maxValue = ADCvalueBuffer[i];}
		if(ADCvalueBuffer[i] < minValue ){minValue = ADCvalueBuffer[i];}
	}
		ST7735_XYplotInit("PMF",minValue,maxValue,0,4096);
		ST7735_XYplot(1000,ADCDefaultValues,pmfOccurences);
	
	
}

int calculate_Jitter(){
	
	uint32_t maxValue = UINT_MIN;
	uint32_t minValue = UINT_MAX;
	uint32_t TimeDifferenceBuffer[NUM_SAMPLES];
	
	
	  for(int i =1; i < NUM_SAMPLES; i++)
	{
		uint32_t difference = TimevalueBuffer[i] - TimevalueBuffer[i-1];
		TimeDifferenceBuffer[i] = difference;
		if(TimeDifferenceBuffer[i] > maxValue){maxValue = TimeDifferenceBuffer[i];}
		if(TimeDifferenceBuffer[i] < minValue ){minValue = TimeDifferenceBuffer[i];}
	}
	
	return maxValue - minValue;
	
	
}
int main(void){
  PLL_Init(Bus80MHz);                   // 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
	Timer1_Init();
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      // turn off LED
	
	
  EnableInterrupts();
	
  while(1){
    PF1 ^= 0x02;  // toggles when running in main
		
		while(BufferIndex < NUM_SAMPLES)
		{
			//busy wait
		}
		
		
	
		int jitter = calculate_Jitter();
		
		for(int i =0; i < 4096; i++)
		{
			pmfOccurences[i] = 0;
			ADCDefaultValues[i] = i;
		}
		
		process_Data();
		
		plot_pmf();
		
  }
}

void DelayWait10s(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
		
    while(time){
	  	time--;
    }
    n--;
  }
}

