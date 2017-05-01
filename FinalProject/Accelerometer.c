/* File Name:    Accelerometer.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      01/19/2017
 * Description:  Measures acceleromter data and does stuff with it.
 * 
 * Lab Number: MW 330-500
 * TA: Mahesh
 * Last Revised: 4/07/2017
*/ 

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "Timer4.h"
#include "Accelerometer.h"
#include "Timer5.h"
#include "AccelAvgFIFO.h"
#include "SysTick.h"
#include "Music.h"
#include "Globals.h"

/*
PE0: X-Accel
PE1: Y-Accel
PE2: Z-Accel
*/

#define TIMER4_RELOAD	0x186A00 // Reload value for an interrupt frequency of 50Hz. 1600000
#define BOLUS_END 26 // Need to append 0 at end of bolus array.
#define BOLUS_START 0
#define UPPER_BOUND 200
#define LOWER_BOUND 60
#define MOVE_BOUND 10

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

volatile int16_t Current_Tempo = 0;

uint16_t bolus_index = BOLUS_END;
uint16_t upper_bound = UPPER_BOUND;
//uint16_t lower_bound;

uint32_t threshhold= 3524;
uint32_t xval[100] = {0};
uint32_t yval[100] = {0};
uint32_t zval[100] = {0};

uint32_t xavg;
uint32_t yavg;
uint32_t zavg;

uint32_t zthreshold1 = 1825;
uint32_t zthreshold2 = 1900;

uint32_t accel_buffer[3] = {0,0,0};

extern volatile int prevsteps;
volatile int steps = 0;
volatile int stepsper2sec =0;
volatile int Timer5Time =0;
volatile int Timer4Time =0;
volatile int last_time =0;
volatile int total_time =0;
volatile int BPMsteps =0;
volatile int BPM =0;
int flag =0;

void Calibrate(){};
	
/*
void BPM_Calc()
{
	int current_time = Timer4Time;
	int time_delta =0;
	if(last_time != 0)
	{
		time_delta = (current_time - last_time);
		total_time += time_delta;
	}
	if(total_time > 0)
	{

		//BPM = ((BPMsteps)*1000)/(total_time * 3);
		//if(Playing){
		//	Change_Tempo(BPM);
		//}
		//BPM /= 1000;
	}
	if(total_time > 100){
		total_time = 0;
		BPMsteps = 0;
	}
	last_time = current_time;
	
}
*/

/*
void Calculate_Steps(){
	//Calculate Steps Algorithm pulled from Instructables.com
	uint32_t totalvector[100] = {0};
	uint32_t totalaverage[100] = {0};
	uint32_t xaccl[100] = {0};
	uint32_t yaccl[100] = {0};
	uint32_t zaccl[100] = {0};
	uint32_t zavebuff[100] = {0};
	
	for(int i =0; i < 100; i++)
	{
		  ADC_In321(accel_buffer);
		
		//Code doens't actually do anything - expected to work better with footsteps
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
				BPMsteps = BPMsteps +1;
				BPM_Calc();
				flag=1;
 
				}
				if ((zavg > zthreshold2 && flag ==1)){flag=0;}
			}
				
		}
	
}
*/



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
	data[0] = ADC0_SSFIFO2_R&0xFFF;	 // 3A) read first result
  data[1] = ADC0_SSFIFO2_R&0xFFF;  // 3B) read second result
  data[2] = ADC0_SSFIFO2_R&0xFFF;  // 3C) read third result
  ADC0_ISC_R = 0x0004;             // 4) acknowledge completion
}

// ***************** manage_Bounds ****************
// Dynamically change tempo bounds.
// Inputs:  none
// Outputs: none
void manage_Bounds(){
	if(Current_Tempo > ((upper_bound * 133) / 100)){
		upper_bound = MIN(upper_bound + MOVE_BOUND, UPPER_BOUND);
	} else if(Current_Tempo < ((upper_bound * 3) >> 2)){
		upper_bound = MAX(upper_bound - MOVE_BOUND, LOWER_BOUND);
	}
}

// ***************** control_Tempo ****************
// Change tempo according to latest steps.
// Inputs:  none
// Outputs: none
void control_Tempo(uint8_t stepCheck){
	if(stepCheck){
		bolus_index = BOLUS_START;
	}
	int16_t bolus_val;
	if(bolus_index == BOLUS_END){
		if(Current_Tempo > 0){
			bolus_val = -1;
		} else{
			bolus_val = 0;
		}
	} else{
		bolus_val = (-4 * bolus_index + 50) / 10;
	}
	Current_Tempo = MIN(Current_Tempo + bolus_val, upper_bound);
	Current_Tempo = MAX(Current_Tempo, 0);
	bolus_index = MIN(bolus_index + 1, BOLUS_END);
	manage_Bounds();
	Mess_With_Tempo(Current_Tempo);
}

void ADC_Read(){
	uint32_t accelData[3];
	ADC_In321(accelData);
	uint8_t stepCheck = Acc_Fifo_Put(accelData[0], accelData[1], accelData[2]);
	control_Tempo(stepCheck);
}

void Accel_Init(void) {
	GPIO_Init();
	Acc_Fifo_Init();
	ADCin_Init();
	Timer4_Init(&ADC_Read,TIMER4_RELOAD);
	//Timer5_Init(320000000);//Every 2 seconds
	//Reserve PA0 to debug init code in the final PCB
	
}

void Accel_Test() {
}

/* Old
void Accel_Test() {
		uint32_t accel[3] = {0,0,0};
    ADC_In321(accel);
		ST7735_SetTextColor(ST7735_BLACK);
    ST7735_DrawStringBG(0,4,"Accelrom X:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,4);  ST7735_OutUDec(accel[0]); ST7735_OutChar('\n');
    ST7735_DrawStringBG(0,5,"Accelrom Y:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,5);  ST7735_OutUDec(accel[1]); ST7735_OutChar('\n');
    ST7735_DrawStringBG(0,6,"Accelrom Z:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,6);  ST7735_OutUDec(accel[2]); ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,7,"NumberStep:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,7);  ST7735_OutUDec(steps); ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,8,"Stepsper4s:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,8);  ST7735_uBinOut8(stepsper2sec); ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,9,"Beatpermin:     ",ST7735_BLACK,ST7735_WHITE); ST7735_SetCursor(12,9);  ST7735_OutUDec(BPM); ST7735_OutChar('\n');
  
}
*/

