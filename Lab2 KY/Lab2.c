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
#include <stdio.h>
#include <stdint.h>
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "fixed.h"
#include "Timer1.h"
#include "Timer2.h"
#include "Timer3.h"


#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
#define NUM_SAMPLES 1000
#define MAX_ADC 4096	
#define UINT_MAX  4294967295
#define UINT_MIN 0


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t ADCvalueBuffer[NUM_SAMPLES];//Debugging Dump Number 1
volatile uint32_t BufferIndex =0;
volatile uint32_t pmfOccurences[MAX_ADC];
volatile uint32_t ADCsampleTimeBuffer[NUM_SAMPLES];
volatile uint32_t ADCtimeDifferenceBuffer[NUM_SAMPLES - 1];
volatile uint32_t maxTimeDifference = UINT_MIN;
volatile uint32_t minTimeDifference = UINT_MAX;
volatile uint32_t Switch1 = 0;

// 180 points on a circle of radius 2.000
int32_t CircleXbuf[180] = { 2000, 1999, 1995, 1989, 1981, 1970, 1956, 1941, 1923, 1902, 1879, 1854, 1827, 1798, 1766, 1732, 1696, 1658, 1618, 1576, 1532, 1486, 1439, 1389, 1338, 1286, 1231, 1176, 1118, 1060, 1000, 939, 877, 813, 749, 684, 618, 551, 484, 416, 347, 278, 209, 140, 70, 0, -70, -140, -209, -278, -347, -416, -484, -551, -618, -684, -749, -813, -877, -939, -1000, -1060, -1118, -1176, -1231, -1286, -1338, -1389, -1439, -1486, -1532, -1576, -1618, -1658, -1696, -1732, -1766, -1798, -1827, -1854, -1879, -1902, -1923, -1941, -1956, -1970, -1981, -1989, -1995, -1999, -2000, -1999, -1995, -1989, -1981, -1970, -1956, -1941, -1923, -1902, -1879, -1854, -1827, -1798, -1766, -1732, -1696, -1658, -1618, -1576, -1532, -1486, -1439, -1389, -1338, -1286, -1231, -1176, -1118, -1060, -1000, -939, -877, -813, -749, -684, -618, -551, -484, -416, -347, -278, -209, -140, -70, 0, 70, 140, 209, 278, 347, 416, 484, 551, 618, 684, 749, 813, 877, 939, 1000, 1060, 1118, 1176, 1231, 1286, 1338, 1389, 1439, 1486, 1532, 1576, 1618, 1658, 1696, 1732, 1766, 1798, 1827, 1854, 1879, 1902, 1923, 1941, 1956, 1970, 1981, 1989, 1995, 1999
};
int32_t CircleYbuf[180] = {0, 70, 140, 209, 278, 347, 416, 484, 551, 618, 684, 749, 813, 877, 939, 1000, 1060, 1118, 1176, 1231, 1286, 1338, 1389, 1439, 1486, 1532, 1576, 1618, 1658, 1696, 1732, 1766, 1798, 1827, 1854, 1879, 1902, 1923, 1941, 1956, 1970, 1981, 1989, 1995, 1999, 2000, 1999, 1995, 1989, 1981, 1970, 1956, 1941, 1923, 1902, 1879, 1854, 1827, 1798, 1766, 1732, 1696, 1658, 1618, 1576, 1532, 1486, 1439, 1389, 1338, 1286, 1231, 1176, 1118, 1060, 1000, 939, 877, 813, 749, 684, 618, 551, 484, 416, 347, 278, 209, 140, 70, 0, -70, -140, -209, -278, -347, -416, -484, -551, -618, -684, -749, -813, -877, -939, -1000, -1060, -1118, -1176, -1231, -1286, -1338, -1389, -1439, -1486, -1532, -1576, -1618, -1658, -1696, -1732, -1766, -1798, -1827, -1854, -1879, -1902, -1923, -1941, -1956, -1970, -1981, -1989, -1995, -1999, -2000, -1999, -1995, -1989, -1981, -1970, -1956, -1941, -1923, -1902, -1879, -1854, -1827, -1798, -1766, -1732, -1696, -1658, -1618, -1576, -1532, -1486, -1439, -1389, -1338, -1286, -1231, -1176, -1118, -1060, -1000, -939, -877, -813, -749, -684, -618, -551, -484, -416, -347, -278, -209, -140, -70
};

void DelayWait2s(uint32_t n){uint32_t volatile time;
  while(n){
    time = 2*72724000*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.
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
	PF2 ^= 0x04;                   // profile
  PF2 ^= 0x04;                   // profile
	if(BufferIndex < NUM_SAMPLES){
            ADCvalueBuffer[BufferIndex] = ADC0_InSeq3();
            ADCsampleTimeBuffer[BufferIndex] = TIMER1_TAR_R;
						if(BufferIndex > 0){
							if((ADCsampleTimeBuffer[BufferIndex - 1] - ADCsampleTimeBuffer[BufferIndex]) > maxTimeDifference){
								// Keep track of maxTimeDifference for jitter calculation.
                maxTimeDifference = ADCsampleTimeBuffer[BufferIndex - 1] - ADCsampleTimeBuffer[BufferIndex];
							}
							if((ADCsampleTimeBuffer[BufferIndex - 1] - ADCsampleTimeBuffer[BufferIndex]) < minTimeDifference){
								// Keep track of minTimeDifference for jitter calculation
								minTimeDifference = ADCsampleTimeBuffer[BufferIndex - 1] - ADCsampleTimeBuffer[BufferIndex];
							}
						}
            BufferIndex++;
	}
	PF2 ^= 0x04;                   // profile
}

void reset_Processing(){
	
    BufferIndex = 0;
    for(int i =0; i < MAX_ADC; i++)
    {
        pmfOccurences[i] = 0;
    }
		minTimeDifference = UINT_MAX;
		maxTimeDifference = UINT_MIN;
	
}

void init_PMF(){
    int maxADCValue = 0;
    int minADCValue = MAX_ADC;
    int maxCountValue = 0;


    for(int i =0; i < NUM_SAMPLES; i++)
    {
        if(ADCvalueBuffer[i] > maxADCValue){maxADCValue = ADCvalueBuffer[i];}
        if(ADCvalueBuffer[i] < minADCValue){minADCValue = ADCvalueBuffer[i];}

    }

    for(int i =0; i < NUM_SAMPLES; i++)
    {
        if(ADCvalueBuffer[i] < 4096)
        {
            pmfOccurences[ADCvalueBuffer[i]] += 1;			
            if(pmfOccurences[ADCvalueBuffer[i]] > maxCountValue){maxCountValue = pmfOccurences[ADCvalueBuffer[i]];}
        }

    }
    ST7735_XYplotInit("PMF",minADCValue,maxADCValue,0,maxCountValue);
	
}

void process_Jitter()
{
		// max/minTimeDifference in units of 12.5 nanoseconds. Jitter in units of ns.
		int32_t jitter = (int32_t) ((maxTimeDifference - minTimeDifference) * 125) / 10;
		ST7735_SetCursor(0,0);
		ST7735_OutString("\nJitter: "); 
		ST7735_sDecOut3(jitter);
		ST7735_OutString("us");
	
}

void init_All(){
    PLL_Init(Bus80MHz);                   // 80 MHz
    SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
    ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
    Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
    GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
    GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
    GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
    GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
    GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
    ST7735_InitR(INITR_REDTAB);
    PF2 = 0;                      // turn off LED

    //Timer1_Init();
    //Timer2_Init(7920); // 7920 / 80MHz = 99us
		//Timer3_Init(7900); // 	^
	
	
}

void test_Them_Lines(void){
		int colors[7] = { ST7735_BLUE, ST7735_RED, ST7735_GREEN, ST7735_CYAN, ST7735_MAGENTA, ST7735_YELLOW, ST7735_WHITE};
		ST7735_XYplotInit("Circle",-2500, 2500, -2500, 2500); //convert maxxX and maxxY to thousands based on resoltuion
		ST7735_Translate(180, (int32_t*) CircleXbuf, (int32_t*) CircleYbuf); // Translate circle to screen for line drawing.
	
		for(int i = 0; i < 180; i++){ // Draw lines in the shape of a circle.
			ST7735_Line(64, 80, CircleXbuf[i], CircleYbuf[i], colors[i % 6]);
		}
		/*
    ST7735_Line(0,0,127,159,ST7735_MAGENTA); //Right Diagonal
		ST7735_Line(7, 90, 100, 83, ST7735_RED);
		ST7735_Line(120, 7, 33, 140, ST7735_BLUE);
    ST7735_Line(0,159,127,0,ST7735_CYAN); //Left Diagonal
    ST7735_Line(50,0,50,159,ST7735_YELLOW);//Vertical
    ST7735_Line(0,50,159,50,ST7735_GREEN);//Horizontal
	*/
}

int main(void){
    init_All();
	
		test_Them_Lines(); // Test line-drawing by drawing lines from a central point to the perimeter of a circle.
    
    DelayWait2s(1);
    
    EnableInterrupts();
    reset_Processing();
		
    while(1){
        //ADC0_SAC_R = 0;
        //ADC0_SAC_R = ADC_SAC_AVG_4X;
        //ADC0_SAC_R = ADC_SAC_AVG_16X;
        ADC0_SAC_R = ADC_SAC_AVG_64X;
		
	
        while(BufferIndex < NUM_SAMPLES){
            //PF1 ^= 0x02;  // toggles when running in main
						PF1 = (PF1*12345678)/1234567+0x02;
					
        }	
		
        if(BufferIndex >= NUM_SAMPLES)
        {
                init_PMF();
                for(int i =0; i < 4096; i++)
                {
                        ST7735_PlotBarXY(i,pmfOccurences[i]);
                }
                process_Jitter();
        }
        reset_Processing();
  }
}



