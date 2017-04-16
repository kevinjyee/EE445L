// Lab10.c
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
#include "tach.h"
#include "switch.h"
#include "motor.h"
#include "ST7735.h"
#include "FIFO.h"
#include "UART.h"

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


#define SW1	0x01 //PE2
#define SW2 0x02 //PE3

#define PWM_PERIOD 		40000
#define CONVERSION_CONSTANT 100


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

extern volatile uint32_t LastE;
volatile uint32_t current_duty = PWM_PERIOD/2;


volatile uint16_t offset = 50;

uint16_t minRPS = 0;
uint16_t maxRPS = 400;
volatile uint16_t pwmcurrentRPS = 200;
volatile uint16_t tachcurrentRPS = 200;

void Init_All(void)
{
	PLL_Init(Bus80MHz);              // 80 MHz clock
	ST7735_InitR(INITR_REDTAB);
	Switch_Init();
	Motor_Init();
 Tach_Init();            // initialize 32-bit timer0A in capture mode
	UART_Init();
  EnableInterrupts();
}


/*
Note to self: 50% duty cycle corressponds to 20 RPS 
*/

uint16_t modify_Speed(void)
{
	uint32_t input = 0x00;
	if(Fifo_Get(&input))
	{
		if(input == SW1)
		{
			if(pwmcurrentRPS <= maxRPS)
			{
				pwmcurrentRPS += offset;
			}
		}
		else if(input == SW2)
		{
			if(pwmcurrentRPS >= minRPS)
			{
				pwmcurrentRPS -= offset;
			}
		}
	Set_Motor_Speed(pwmcurrentRPS*CONVERSION_CONSTANT-1); 
	}
	return pwmcurrentRPS;
}


void Debug(pwmcurrentRPS,tachcurrentRPS)
{
	UART_OutString("PWM_Speed: ");
	UART_OutUDec(pwmcurrentRPS);
	UART_OutString("\n");
	
	UART_OutString("Tach_Speed: ");
	UART_OutUDec(tachcurrentRPS);
	UART_OutString("\n");
}

int main(void){           
	Init_All();
  while(1){
		pwmcurrentRPS = modify_Speed();
		tachcurrentRPS = Tach_Read();
		//Debug(pwmcurrentRPS,tachcurrentRPS);
	}
}
