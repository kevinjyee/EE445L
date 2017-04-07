/* File Name:    Lab5.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      02/069/2017
 * Description:  Main functions to run Music Player lab.
 *               
 * 
 * Lab Number: MW 330-500
 * TA: Mahesh
 * 
 * Hardware Configurations:
 * ST7735R LCD:
 *     Backlight    (pin 10) connected to +3.3 V
 *     MISO         (pin 9) unconnected
 *     SCK          (pin 8) connected to PA2 (SSI0Clk)
 *     MOSI         (pin 7) connected to PA5 (SSI0Tx)
 *     TFT_CS       (pin 6) connected to PA3 (SSI0Fss)
 *     CARD_CS      (pin 5) unconnected
 *     Data/Command (pin 4) connected to PA6 (GPIO)
 *     RESET        (pin 3) connected to PA7 (GPIO)
 *     VCC          (pin 2) connected to +3.3 V
 *     Gnd          (pin 1) connected to ground
 *		 Down Switch				 PE0
 *		 Up Switch					 PE1
 *		 Select Switch			 PE2
 *		 Main Switch				 PE3
 *		 Speaker Input			 PB6
 
 */
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
#include "Heap.h"
#include "TitleScreen.h"
#include "Accelerometer.h"
#include "SongMenu.h"


#define SYSTICK_RELOAD	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.

#define TOGGLE_PLAY 0x40
#define CHANGE_SONG 0x80
#define REWIND 			0xF0

#define PLAY 				1
#define PAUSE 			0

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//volatile uint32_t Switch1 = 0;
volatile unsigned long LastE = 0; 
char Play_Toggled;

/*Function: DelayWait2ms
*
*/
void DelayWait2ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 7272400*4/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

/*Function: init_All
*Initializes all the Timers and switches
*/
void init_All(){
	PLL_Init(Bus50MHz);                   // 50 MHz
	Switch_Init();
	Accel_Init();
	ST7735_InitR(INITR_REDTAB);
  ST7735_DrawBitmap(0,159,TitleScreen2,128,160);
	DelayWait2ms(10);
	ST7735_FillScreen(ST7735_WHITE);
	SongMenu_Init();
	DAC_Init(0);
	Heap_Init();
  
}

void Buttons_Test(){
	if(Switch_In() == 0x01)
	{
		
		ST7735_OutString("Down");
	}
	if(Switch_In() == 0x02)
	{
		
		ST7735_OutString("Up");
	}
	if(Switch_In() == 0x04)
	{
		
		ST7735_OutString("Left");
	}
	if(Switch_In() == 0x08)
	{
		
		ST7735_OutString("Right");
	}
	if(Switch_In() == 0x10)
	{
		
		ST7735_OutString("Select");
	}
	
}

int main(void){
  init_All();
	EnableInterrupts();
	uint32_t current_state = 0x00;	
  uint32_t input,lastinput = 0x00;
	while(1){
		Buttons_Test();
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




/*
int main(void){
	init_All();
	
	while(1)
	{
	Accel_Test();
	}
	
}
*/

