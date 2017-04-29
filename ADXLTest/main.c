// main.c
// Runs on LM4F120/TM4C123
// UART runs at 115,200 baud rate 
// Daniel Valvano
// May 3, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
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


#include <stdint.h> // C99 variable types
#include "ADCSWTrigger.h"
#include "uart.h"
#include "PLL.h"
#include "Accelerometer.h"
#include <math.h>
#include "Switch.h"
#include "../inc/tm4c123gh6pm.h"
#include "AccelAvgFIFO.h"
#include "ST7735.h"
#include "DAC.h"
#include "heap.h"
#include "Music.h"

#define PF0       (*((volatile uint32_t *)0x40025004))
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PF4       (*((volatile uint32_t *)0x40025040))
#define LEDS      (*((volatile uint32_t *)0x40025038))
#define RED       0x02
#define BLUE      0x04
#define GREEN     0x08
#define SWITCHES  (*((volatile uint32_t *)0x40025044))
#define SW1       0x10                      // on the left side of the Launchpad board
#define SW2       0x01                      // on the right side of the Launchpad board

extern volatile uint32_t stepCount;
extern volatile uint32_t XThresh, YThresh, ZThresh;
extern volatile uint32_t XAvg, YAvg, ZAvg;

int main(void){ int32_t data; uint32_t status;
  PLL_Init(Bus80MHz);   // 80 MHz
  UART_Init();              // initialize UART device
	int collect = 0;
  //ADC0_InitSWTriggerSeq3_Ch9();
	Accel_Init();
	Switch_Init();           // PA5 is input
	DAC_Init(0);
	Heap_Init();
	ST7735_InitR(INITR_REDTAB);
  status = Switch_Input(); // 0x00 or 0x20
  status = Switch_Input(); // 0x00 or 0x20
  
  Board_Init();             // initialize PF0 and PF4 and make them inputs
                            // make PF3-1 out (PF3-1 built-in LEDs)
  GPIO_PORTF_DIR_R |= (RED|BLUE|GREEN);
                              // disable alt funct on PF3-1
  GPIO_PORTF_AFSEL_R &= ~(RED|BLUE|GREEN);
                              // enable digital I/O on PF3-1
  GPIO_PORTF_DEN_R |= (RED|BLUE|GREEN);
                              // configure PF3-1 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF000F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;     // disable analog functionality on PF
	uint32_t accel[3] = {0,0,0};
	UART_OutString("\nADXL data: ");
	Play();
  while(1){
		ST7735_DrawStringBG(0,4,"Step Count:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(12,4);  
		ST7735_OutUDec(stepCount); 
		ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,5,"Averages:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(2,6);  
		ST7735_OutUDec(XAvg);
		ST7735_OutChar(' ');
		ST7735_OutUDec(YAvg);
		ST7735_OutChar(' ');
		ST7735_OutUDec(ZAvg);
		ST7735_OutChar(' ');
		ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,7,"Thresholds:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(2,8);  
		ST7735_OutUDec(XThresh);
		ST7735_OutChar(' ');
		ST7735_OutUDec(YThresh);
		ST7735_OutChar(' ');
		ST7735_OutUDec(ZThresh);
		ST7735_OutChar(' ');
		ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,9,"NextStepTooSoon:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(2,10);  
		ST7735_OutUDec(NextStepTooSoon);
		ST7735_DrawStringBG(0,11,"DetectionAxis:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(2,12);  
		if(DetectionAxis == 1){
			ST7735_OutChar('X');
		} else if(DetectionAxis == 2){
			ST7735_OutChar('Y');
		} else if(DetectionAxis == 3){
			ST7735_OutChar('Z');
		}
		
		/*
		status = Board_Input();
		if((status & 0x01) && (status != 0x11)){
			collect ^= 1;
		}
		if(collect){
    //data = ADC0_InSeq3();
			UART_OutString("\n(");
			ADC_In321(accel);
			UART_OutUDec(accel[0]);
			UART_OutString(",");
			UART_OutUDec(accel[1]);
			UART_OutString(",");
			UART_OutUDec(accel[2]);
			uint32_t mag = 0;
			UART_OutString(",");
			UART_OutUDec(mag);
			UART_OutString(")");
		}
		*/
  }
}


