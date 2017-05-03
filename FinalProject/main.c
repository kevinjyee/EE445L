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
#include "AccelAvgFIFO.h"
#include "Globals.h"
#include "SendSteps.h"

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

char REQUESTT[] ="GET /query?city=Austin%2C%20Texas&id=Kevin%20and%20Stefan&greet=esptest4 HTTP/1.1\r\nUser-Agent: Keil\r\nHost: ee445l-kjy252.appspot.com\r\n\r\n";
// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID

void extractStep(char* Recvbuff, char* tempbuffer){
	const char TEMP[] = "<pre>";
	const char COMMA[] = "}";
	char* start = strstr(Recvbuff,TEMP);
	char* end = strstr(start,COMMA);
	start += strlen(TEMP)*sizeof(char)+1;
	int i =0;
	while(*start != *end)
	{
		if(*start != '\"' && *start != 0)
		{
		tempbuffer[i++] = *start;
		
		}
		start ++;
	}
	}

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
		//ESP8266FIFOtoBuffer();
		//char* buffer;
		//buffer = get_SearchString();
   
		//char tempbuffer[50] = " ";
		//extractStep(buffer, tempbuffer);
	//if(tempbuffer[0] >= '0' && tempbuffer[0] <= '9')
	//{
		//Step_Count = atoi(tempbuffer);
	//}
	//else
	//{
		//Step_Count = 0;
	//}
	
	Switch_Init();
	Accel_Init();
	SongMenu_Init();
	DAC_Init(0);
	DAC_InitB(0);
	Heap_Init();
	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_WHITE);
	EnableInterrupts();
	
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






