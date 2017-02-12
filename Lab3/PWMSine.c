// PWMSine.c
// Runs on TM4C123
// Use Timer0A in periodic mode to request interrupts at a particular period.
// In those interrupts, change the PWM duty cycle to produce a sine wave.
// Daniel Valvano
// January 26, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 6.8, Section 6.3.2
   Program 8.7, Example 8.4

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Program 8.4, Section 8.3

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

// oscilloscope connected to PWM0A/PB6 for PWM output
// PF2 toggles on each timer interrupt (flash at frequency of sine wave * 32 / 2)

#include <stdint.h>
#include "PLL.h"
#include "PWM.h"
#include "timer0A.h"
#include "PWMSine.h"

#define PF2                     (*((volatile uint32_t *)0x40025010))
#define GPIO_PORTF_DIR_R        (*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_DEN_R        (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile uint32_t *)0x4002552C))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R5      0x00000020  // GPIO Port F Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R5        0x00000020  // GPIO Port F Peripheral Ready

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t StartCritical (void);// previous I bit, disable interrupts
void EndCritical(uint32_t sr);// restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

#define C3 		0
#define C3Sh 	1
#define D3		2
#define D3Sh	3
#define E3		4
#define F3		5
#define F3Sh	6
#define G3 		7
#define G3Sh	8
#define A3		9
#define A3Sh	10
#define B3		11
#define C4 		12
#define C4Sh 	13
#define D4		14
#define D4Sh	15
#define E4		16
#define F4		17
#define F4Sh	18
#define G4 		19
#define G4Sh	20
#define A4		21
#define A4Sh	22
#define B4		23
#define C5		24
#define C5Sh	25
#define D5		26
#define D5Sh  27
#define E5    28
#define F5    29
#define F5Sh  30
#define G5		31
#define C6		32
#define C7		33	
#define REST  34
#define NUM_NOTES 35
#define IMPERIAL_MARCH_LENGTH 97
#define MORNING_MOOD_LENGTH 96
#define NEW_BARK_LENGTH 132
#define BEEP_LENGTH 24
#define SIREN_LENGTH 2


const uint16_t Notes[NUM_NOTES] = { //C3-G5
	19111, 18039, 17026, 16071, 15169, 14318, 13513, 12755, 12039, 11364,
					10726, 10124, 9555, 9019, 8513, 8035, 7584, 7159, 6757, 6378, 
					6020, 5682, 5363, 5061, 4778, 4510, 4257, 4018, 3792, 3579, 
					3378, 3189, 2389, 1194, 65535
};

const uint16_t Wave[32] = {
  125,143,159,175,189,200,208,213,215,213,208,
  200,189,175,159,143,125,107,91,75,61,50,
  42,37,35,37,42,50,61,75,91,107
};

const uint16_t Square_Wave[32] = {
	215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215, 215,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t Imperial_March[IMPERIAL_MARCH_LENGTH] = { // Tempo 3
	A3, A3, REST, A3, A3, REST, A3, A3, REST, F3, F3, C4, A3, A3, REST, F3, F3, C4, A3, A3, A3, A3, REST, REST,
	E4, E4, REST, E4, E4, REST, E4, E4, REST, F4, F4, C4, G3Sh, G3Sh, G3Sh, F3, F3, C4, A3, A3, A3, A3, REST, REST,
	A4, A4, REST, A3, A3, A3, A4, A4, A4, G4Sh, G4Sh, G4, F4Sh, F4, F4Sh, F4Sh, REST, REST, A3Sh, D4Sh, D4Sh, D4Sh, D4, D4,
	C4Sh, C4, B3, C4, C4, REST, F3, G3Sh, G3Sh, G3Sh, F3, F3, G3Sh, C4, C4, C4, A3, A3,
	C4, E4, E4, E4, E4, E4, E4
};

const uint8_t Morning_Mood[MORNING_MOOD_LENGTH] = { // Tempo 3
	C4, C4, A3, A3, G3, G3, F3, F3, G3, G3, A3, A3, C4, C4, A3, A3, G3, G3, F3, F3, G3, A3, G3, A3, C4, C4, A3, A3,
	C4, C4, D4, D4, A3, A3, D4, D4, C4, C4, A3, A3, G3, G3, F3, F3, F3, F3, REST, REST,
	C4, C4, A3, A3, G3, G3, F3, F3, G3, G3, A3, A3, C4, C4, A3, A3, G3, G3, F3, F3, G3, A3, G3, A3, C4, C4, A3, A3,
	C4, C4, D4, D4, A3, A3, D4, D4, E4, E4, C4Sh, C4Sh, B3, B3, A3, A3, A3, A3, REST, REST
};

const uint8_t New_Bark_Town[NEW_BARK_LENGTH] = {
	D4, D4, E4, E4, F4Sh, F4Sh, F4Sh, F4Sh, A4, A4, A4, A4, G4, G4, F4Sh, F4Sh, E4, E4, G4, G4,
	F4Sh, F4Sh, F4Sh, F4Sh, F4Sh, F4Sh, D4, D4, A3, A3, A3, A3, REST, REST, G3, A3, B3, B3, B3, B3,
	D4, D4, D4, D4, E4, E4, D4, D4, C4Sh, C4Sh, D4, D4, E4, E4, E4, E4, E4, E4, F4Sh, F4Sh, E4, E4, E4, E4,
	REST, REST, D4, E4, F4Sh, F4Sh, F4Sh, F4Sh, A4, A4, A4, A4, A4Sh, A4Sh, A4, A4, G4, G4, A4Sh, A4Sh,
	A4, A4, A4, A4, A4, A4, C5Sh, C5Sh, D5, D5, D5, D5, REST, REST, E4, F4Sh, G4, G4, G4, G4, G4, G4,
	A4, A4, B4, B4, B4, B4, B4, B4, B4, B4, A4, A4, A4, A4, A4, A4, G4, F4Sh, E4, E4, E4, E4, E4, E4, E4, E4
};

const uint8_t BEEP_BEEP_BEEP_BEEP[24] = { // Tempo 1
	C7, C7, C7, REST, C7, C7, C7, REST, C7, C7, C7, REST, C7, C7, C7, REST, REST, REST, REST, REST, REST, REST, REST, REST
};

const uint8_t WEE_WOO_WEE_WOO[SIREN_LENGTH] = {
	C6, G5
};

uint8_t song_Index = 0;
uint8_t previous_Song = -1;


void OutputSineWave(void){
  static uint8_t index = 0;        // counting index of output sequence
  PWM0A_Duty(Square_Wave[index]);         // output next value in sequence
  index = (index + 1)&0x1F;        // increment counter
  //PF2 ^= 0x04;
}

void PMWSine_Init(uint8_t song_Id){
  //PLL_Init(Bus50MHz);              // 50 MHz
                                   // 1) activate clock for Port F
	
	if(song_Id != previous_Song){ // Reset song_Index for a new song.
		song_Index = 0;
		previous_Song = song_Id;
	}
	/*
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
                                   // allow time to finish activating
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5)==0){};
                                   // 2) no need to unlock PF2
  GPIO_PORTF_PCTL_R &= ~0x00000F00;// 3) configure PF2 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x04;     // 4) disable analog function on PF2
  GPIO_PORTF_DIR_R |= 0x04;        // 5) make PF2 out (PF2 built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;     // 6) disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;        // 7) enable digital I/O on PF2
	*/
  PWM0A_Init(250, 125);            // initialize PWM0A, 100kHz, 50% duty
	switch(song_Id){
		case 0: Timer0A_Init(&OutputSineWave, Notes[BEEP_BEEP_BEEP_BEEP[song_Index]]);
			song_Index = (song_Index + 1) % BEEP_LENGTH;
			break;
		case 1: Timer0A_Init(&OutputSineWave, Notes[WEE_WOO_WEE_WOO[song_Index]]);
			song_Index = (song_Index + 1) % SIREN_LENGTH;
			break;
		case 2: Timer0A_Init(&OutputSineWave, Notes[Imperial_March[song_Index]]);
			song_Index = (song_Index + 1) % IMPERIAL_MARCH_LENGTH;
			break;
		case 3: Timer0A_Init(&OutputSineWave, Notes[Morning_Mood[song_Index]]);
			song_Index = (song_Index + 1) % MORNING_MOOD_LENGTH;
			break;
		case 4: Timer0A_Init(&OutputSineWave, Notes[New_Bark_Town[song_Index]]);
			song_Index = (song_Index + 1) % NEW_BARK_LENGTH;
			break;
	}
}
