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
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "fixed.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOqueue.h"
#include "FSM.h"
#include "Clock.h"
#include "LCD.h"
#include "PWM.h"
#include "Timer0A.h"
#include "Timer3.h"


#define PA3							(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2             (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1             (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0							(*((volatile uint32_t *)0x40004004)) // Down switch
#define SYSTICK_RELOAD	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.

#define HOUR 0
#define MIN 1 
#define SEC 2
#define MER 3
#define ALL 4
#define NUMALARMS 8

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t Switch1 = 0;
volatile uint32_t Time = 0; // (Meridian)hh0mm0ss
volatile uint32_t AlarmTime =0;
volatile uint8_t SelectSeconds;
volatile uint8_t SelectMinutes;
volatile uint8_t SelectHours;
volatile uint8_t SelectMeridian;
volatile unsigned long LastE = 0; 

uint32_t Tempo[10] = { 60, 80, 100, 120, 140, 160, 180, 200, 220, 240 };

volatile bool animateAlarm = false;
volatile bool resetClock;
extern int AlarmOn;


// Retrieves seconds, minutes, and hour from Time global variable in thread-safe fashion.
// Consider switching to bit operations to avoid overhead of division.
void getTime(uint8_t* meridian, uint8_t* hours, uint8_t* minutes, uint8_t* seconds){
	uint32_t time = Time;
	*seconds = time % 100;
	time = time / 1000;
	*minutes = time % 100;
	time = time / 1000;
	*hours = (time / 1000000) % 100;
	time = time / 100;
	*meridian = time % 10;
	
}

void DelayWait2ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 7272400*4/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

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

void sound_On(){
	Timer3_Init(Tempo[4]);
}

void sound_Off(){
	Disable_Timer3();
	Disable_PWM();
}

void init_All(){
	PLL_Init(Bus50MHz);                   // 50 MHz
	init_switchmain();
	
	ST7735_InitR(INITR_REDTAB);
  SysTick_Init(SYSTICK_RELOAD);
	//Timer1_Init();
	
}

uint32_t find_minAlarm(){
	uint32_t min =0XFFFFFFFF;
  int index = -1;
	bool foundalarm = false;
	for(int i =0; i < NUMALARMS; i++)
	{
		if(AlarmONOFFArray[i] == 1)
		{
			if(AlarmTimeArray[i] - Time < min)
			{
			min = AlarmTimeArray[i];
			index = i;
			}
			foundalarm = true;	
		}		
	}
	if(foundalarm)
	{
		AlarmOn = 1;
	}
	else{
		AlarmOn = 0;
	}
return index;
}

void check_Alarm(uint32_t current_state){
	if(current_state == 0x00)
		{
		draw_Time(); // Start updating time.
			uint32_t minalarm = find_minAlarm();
			if(AlarmOn == 1)
			{
				ST7735_OutString(" Alarm");
				ST7735_OutUDec(minalarm);
				ST7735_OutString("On");
			}
		}
		for(int i =0; i < NUMALARMS; i++)
		{
			if(Time == AlarmTimeArray[i] && AlarmONOFFArray[i] == 1)
		{
			sound_On();
			AlarmONOFFArray[i] = 0;
			animateAlarm = true;
		}
			
		}
}


int main(void){
  init_All();
	draw_Clock();
	EnableInterrupts();
	uint32_t current_state = 0x00;	
  uint32_t input,lastinput = 0x00;
	uint8_t alarm_triggered = 0;
	int mute = 0;
	while(1){
		check_Alarm(current_state);
				
			while(animateAlarm && !mute)
			{
				alarm_triggered = 1;
				mute = animate_Clock();		
			}
			if(alarm_triggered){
				mute = 0;
				animateAlarm = false;
				AlarmOn = 0;
			  sound_Off();
				clear_Screen();
				if(current_state == 0x00){
					redrawHands = 1;
					draw_Clock();
					draw_Time();
				}
				alarm_triggered = 0;
				resetClock = true;
			}
			
		
		if(Fifo_Get(&input))
		{
			current_state = Next_State(current_state, input);			
			lastinput = input;
		 
		}
		else
		{
			current_state = Next_State(current_state,0x00);	
		}
  }
}




