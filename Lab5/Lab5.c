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

#define PA3							(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2             (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1             (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0							(*((volatile uint32_t *)0x40004004)) // Down switch
#define SYSTICK_RELOAD	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.

#define TOGGLE_PLAY 1
#define CHANGE_SONG 2
#define REWIND 			3

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
	ST7735_InitR(INITR_REDTAB);
}

int main(void){
  init_All();
	EnableInterrupts();
	uint32_t current_state = 0x00;	
  uint32_t input,lastinput = 0x00;
	while(1){
			
		if(Fifo_Get(&input))
		{
			current_state = Next_State(current_state, input);			
			lastinput = input;
		 
		}
		else
		{
			current_state = Next_State(current_state,0x00);	
		}
		switch(current_state){
			case TOGGLE_PLAY:
				if(Play_Toggled){
					Play();
				} else{
					Pause();
				}
			case CHANGE_SONG:
				Change_Song();
			case REWIND:
				Rewind();
			default:
				break;
		}
  }
}




