#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOqueue.h"
#include "FSM.h"
#include "LCD.h"
#include "Timer0A.h"
#include "Timer1.h"
#include "Timer3.h"
#include "Music.h"
#include "DAC.h"



/*
PE0: X-Accel
PE1: Y-Accel
PE2: Z-Accel
*/



void Accel_Init(void) {
	
	volatile uint32_t delay;
	SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; // 1) activate clock for Port E
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;

	GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R&0xFFFFF000;
  GPIO_PORTE_DIR_R &= ~0x07;      // make PE 2,1,0 input
  GPIO_PORTE_DIR_R |= 0x08;      // make PE3 output for ST pin for debugging later for Main Switch
  GPIO_PORTE_AFSEL_R |= 0x07;     // enable alternate function on PE 2,1,0 for ADC
  GPIO_PORTE_AFSEL_R &= ~0x08;     // disable alternate function on PE3 for ST pins for debugging later
  GPIO_PORTE_DEN_R &= ~0x07;      // disable digital I/O on PE 2,1,0
  GPIO_PORTE_DEN_R |= 0x08;      // enable digital I/O on PE3
	GPIO_PORTE_PCTL_R = GPIO_PORTE_PCTL_R&0xFFFFF000;
  GPIO_PORTE_AMSEL_R |= 0x07;     // enable analog functionality on PE 2,1,0  
	
	//Block of Code for Debugging Main Switch
	GPIO_PORTE_PDR_R |= 0x08; //Switch 3 
	GPIO_PORTE_IS_R &= ~0x08;
	GPIO_PORTE_IBE_R |= 0x08;
	
	ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority
  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  
	
	ADC0_SSMUX2_R = 0x023;         // 12) set channels for SS2
  ADC0_SSCTL2_R = 0x0060;         // 13) no TS0 D0 IE0 END0 TS1 D1, yes IE2 END2 - 3 bit Accel input
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2 allows for 4 inputs
	
	//Reserve PA0 to debug init code in the final PCB
}

//------------ADC_In321------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: two 12-bit result of ADC conversions
// Samples ADC8 and ADC9 
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// data[0] is Ain3 (PE0) 0 to 4095
// data[1] is Ain2 (PE1) 0 to 4095
// data[2] is Ain1 (PE3)
void ADC_In321(uint32_t data[3]){ 
  ADC0_PSSI_R = 0x0004;            // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};   // 2) wait for conversion done
	data[0] = ADC0_SSFIFO2_R&0xFFF;	
  data[1] = ADC0_SSFIFO2_R&0xFFF;  // 3A) read first result
 // data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3B) read second result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}

void Accel_Test() {
		uint32_t accel[3] = {0,0,0};
    ADC_In321(accel);
    ST7735_SetCursor(0,0);
    ST7735_OutString("Accelrom X:     "); ST7735_SetCursor(12,4);  ST7735_OutUDec(accel[0]); ST7735_OutChar('\n');
    ST7735_OutString("Accelrom Y:     "); ST7735_SetCursor(12,5);  ST7735_OutUDec(accel[1]); ST7735_OutChar('\n');
   // ST7735_OutString("Accelrom Z:     "); ST7735_SetCursor(12,6);  ST7735_OutUDec(accel[2]); ST7735_OutChar('\n');
   
  
}

