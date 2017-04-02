#include <stdint.h>
#include <stdbool.h>
#include <math.h>
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
#include "Timer4.h"
#include "Accelerometer.h"


/*
PE0: X-Accel
PE1: Y-Accel
PE2: Z-Accel
*/

#define TIMER4_RELOAD	0x4C4B40 // Reload value for an interrupt frequency of 10Hz. 5000000

uint32_t threshhold= 3524;
uint32_t xval[100] = {0};
uint32_t yval[100] = {0};
uint32_t zval[100] = {0};

uint32_t xavg;
uint32_t yavg;
uint32_t zavg;

uint32_t zthreshold1 = 1720;
uint32_t zthreshold2 = 1900;

uint32_t accel_buffer[3] = {0,0,0};

int steps,flag =0;

void Calibrate(){};

void Calculate_Steps(){
	//Calculate Steps Algorithm pulled from Instructables.com
	uint32_t totalvector[100] = {0};
	uint32_t totalaverage[100] = {0};
	uint32_t xaccl[100] = {0};
	uint32_t yaccl[100] = {0};
	uint32_t zaccl[100] = {0};
	
	
	for(int i =0; i < 100; i++)
	{
		  ADC_In321(accel_buffer);
			xval[i] = accel_buffer[XACCEL];
			yval[i] = accel_buffer[YACCEL];
			zval[i] = accel_buffer[ZACCEL];
		
			totalvector[i] = sqrt(((xaccl[i]-xavg)* (xaccl[i]-xavg))+ ((yaccl[i] - yavg)*(yaccl[i] - yavg)) + ((zval[i] - zavg)*(zval[i] - zavg)));
			if(i >= 1)
			{
				zavg = (zval[i] + zval[i-1])/2;
				
				totalaverage[i] = (totalvector[i] + totalvector[i-1]) / 2 ;
				if ((zavg < zthreshold1 &&flag ==0))
				{
				steps=steps+1;
				flag=1;
 
				}
				if ((zavg > zthreshold2 && flag ==1)){flag=0;}
			}
			
				
	}
	
}

void GPIO_Init(){
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
	GPIO_PORTE_AMSEL_R &= ~0x08;			//diable for PE3
	//Block of Code for Debugging Main Switch
	GPIO_PORTE_PDR_R |= 0x08; //Switch 3 
	GPIO_PORTE_IS_R &= ~0x08;
	GPIO_PORTE_IBE_R |= 0x08;
}


void ADCin_Init(){
	
		
	ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority
  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  
	
	ADC0_SSMUX2_R = 0x0123;         // 12) set channels for SS2
  ADC0_SSCTL2_R = 0x0600;         // 13) no TS0 D0 IE0 END0 TS1 D1, yes IE2 END2 - 3 bit Accel input
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2 allows for 4 inputs
	
}


void Accel_Init(void) {
	GPIO_Init();
	ADCin_Init();
	Timer4_Init(&Calculate_Steps,TIMER4_RELOAD);
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
  data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3B) read second result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}

void Accel_Test() {
		uint32_t accel[3] = {0,0,0};
    ADC_In321(accel);
		ST7735_SetTextColor(ST7735_BLACK);
    ST7735_DrawStringBG(0,4,"Accelrom X:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,4);  ST7735_OutUDec(accel[0]); ST7735_OutChar('\n');
    ST7735_DrawStringBG(0,5,"Accelrom Y:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,5);  ST7735_OutUDec(accel[1]); ST7735_OutChar('\n');
    ST7735_DrawStringBG(0,6,"Accelrom Z:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,6);  ST7735_OutUDec(accel[2]); ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,7,"NumberStep:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,7);  ST7735_OutUDec(steps); ST7735_OutChar('\n');
  
}
