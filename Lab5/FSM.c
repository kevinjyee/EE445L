#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOQueue.h"
#include "Lab5.h"
#include "LCD.h"

#define PA3						(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2           (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1           (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0					  (*((volatile uint32_t *)0x40004004)) // Down switch


#define TRUE 				1 
#define FALSE 			0

#define TOGGLE_PLAY 1
#define CHANGE_SONG 2
#define REWIND 			3
#define PLAY 				1
#define PAUSE 			0

#define NUMSONGS    3
#define NUM_MODES		3

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

int selectSwitchToggled = FALSE;
extern char Play_Toggled;
volatile int currentSongPos = 0;
volatile int currentMode = 0;
volatile int lastSongPos = -1;

uint32_t MainScreen(uint32_t);
uint32_t ChooseSong(uint32_t);

char BeepingAlarm[17] = {' ', ' ', 'E', 'n', 'v', 'e', 'l', 'o', 'p', 'e', ' ', 'T', 'e', 's', 't', ' ', 0};
char Siren[17] = {' ', ' ', 'L', 'o', 's', 't', ' ', 'W', 'o', 'o', 'd', ' ', ' ', ' ', ' ', ' ', 0};
//char ImperialMarch[17] = {' ', ' ', 'I', 'm', 'p', 'e', 'r', 'i', 'a', 'l', ' ', 'M', 'a', 'r', 'c', 'h', 0};
//char MorningMood[17] = {' ', ' ', 'M', 'o', 'r', 'n', 'i', 'n', 'g', ' ', 'M', 'o', 'o', 'd', ' ', ' ', 0};
char NewBarkTown[17] = {' ', ' ', 'N', 'e', 'w', ' ', 'B', 'a', 'r', 'k', ' ', 'T', 'o', 'w', 'n', ' ', 0};
char* song_Choice[3] = {		
	BeepingAlarm, Siren,  NewBarkTown
};

void Next_State(uint32_t current_state, uint32_t keyInputs)
{
	/*
	switch(current_state){
		case 0x00:
			return MainScreen(keyInputs);
		case 0x01:
			break;
		case 0x02:
			break;
		case 0x03:
			return ChooseSong(keyInputs);
		case 0x04:
			break;
		default:
			return ChooseSong(keyInputs);
	}
	*/
	ChooseSong(keyInputs);
}


void clear_Menu(){
			ST7735_FillRect(0,0,125,10,ST7735_BLACK);
			ST7735_DrawString(0,1,"  Set Time",ST7735_BLACK);
			ST7735_DrawString(0,2,"  Set Alarm",ST7735_BLACK);
			ST7735_DrawString(0,3,"  Choose Song",ST7735_BLACK);
}

uint32_t ChooseSong(uint32_t input)
{
	
	switch(input)
	{
		case 0x01: 
			//MODE switch
			currentMode = (currentMode + 1) % NUM_MODES;
			break;
		case 0x02:
			//Up Switch
			
			currentSongPos = (currentSongPos + 1)%NUMSONGS;
			if(currentSongPos < 0 )
			{
				currentSongPos = NUMSONGS-1; 
			}
			break;
		case 0x04:
			Play_Toggled ^= PLAY;
			if((currentSongPos != lastSongPos) && (Play_Toggled == PLAY)){
				lastSongPos = currentSongPos;
				return CHANGE_SONG;
			} else{
				return TOGGLE_PLAY;
			}
		case 0x08:
			return REWIND;
		
	}
	ST7735_DrawString(0,0,"Song Choices:",ST7735_YELLOW);
	for(int i =0; i < NUMSONGS; i++)
	{
		if(i == currentSongPos)
		{
			ST7735_DrawString(0,i + 1,song_Choice[i],ST7735_CYAN);
		}
		else{
			ST7735_DrawString(0,i + 1,song_Choice[i],ST7735_WHITE);
		}
	}
	return 0;
}
