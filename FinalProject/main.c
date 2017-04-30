//***********************  main.c  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Jonathan Valvano
 Sept 14, 2016
 Out of the box: to make this work you must
 Step 1) Set parameters of your AP in lines 59-60 of esp8266.c
 Step 2) Change line 39 with directions in lines 40-42
 Step 3) Run a terminal emulator like Putty or TExasDisplay at
         115200 bits/sec, 8 bit, 1 stop, no flow control
 Step 4) Set line 50 to match baud rate of your ESP8266 (9600 or 115200)
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/tm4c123gh6pm.h"

#include "pll.h"
#include "UART.h"
#include "esp8266.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOqueue.h"
#include "FSM.h"
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

// prototypes for functions defined in startup.s
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

char Fetch[] = "GET /data/2.5/weather?q=Austin%20Texas&APPID=1bc54f645c5f1c75e681c102ed4bbca4&units=metric HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n";
char REQUESTT[] ="GET /query?city=Austin%2C%20Texas&id=Kevin%20and%20Stefan&greet=esptest3 HTTP/1.1\r\nUser-Agent: Keil\r\nHost: ee445l-kjy252.appspot.com\r\n\r\n";
// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID

void init_All(){
	DisableInterrupts();
	PLL_Init(Bus80MHz);                   // 50 MHz
  ST7735_InitR(INITR_REDTAB);
  ST7735_DrawBitmap(0,159,TitleScreen2,128,160);
	Output_InitESP();
  ESP8266_Init(115200);      // connect to access point, set up as client
  ESP8266_GetVersionNumber();
  ESP8266_GetStatus();
			if(ESP8266_MakeTCPConnection("ee445l-kjy252.appspot.com")){ // open socket in server
      
      ESP8266_SendTCP(REQUESTT);
    }
		ESP8266_CloseTCPConnection();
	Switch_Init();
	Accel_Init();
	

	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_WHITE);
	SongMenu_Init();
	DAC_Init(0);
	Heap_Init();
	
}
int main(void){  
	init_All();
	
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
		
    
  }
}






