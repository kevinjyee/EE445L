// FSM.h
// Runs on LM4F120/TM4C123
// The state machine underlying the soundtrack project.
//	Based on switch input, allows a user to navigate between
//	several screens including a title screen,
//	song selection screen, and options menu.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOQueue.h"
#include "Lab8.h"
#include "LCD.h"
#include "MainMenu.h"
#include "SongMenu.h"
#include "SongScreen.h"
#include "PedometerScreen.h"
#include "Music.h"

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
volatile int currentMode = 0;
volatile int currentSongPos = 0;


SongChoice SongsList[NUMSONGS];

static const uint16_t* dummy = {0x00};
uint32_t Next_State(uint32_t current_state, uint32_t keyInputs)
{
	switch(current_state){
		case 0x00:
			return MainScreen(keyInputs);
		case 0x01:
			return SongMenu(keyInputs);
		case 0x02:
			return PedometerScreen(keyInputs);
		case 0x03:
			return SongScreen(keyInputs, SongsList[currentSongPos].SongName,dummy);
		case 0x04:
			//return SetAlarms(keyInputs);
		default:
			return MainScreen(keyInputs);
	}
}


void Draw_Title(int XTITLE, int YTITLE, int TITLEBORDER,char* title){
	ST7735_SetCursor(0,0);
	ST7735_DrawString(XTITLE,YTITLE,title,ST7735_BLACK);
	ST7735_DrawFastHLine(0,TITLEBORDER,128,ST7735_BLACK);
	
}

void Draw_Options(uint8_t menupos,char* menu_Choice[],uint8_t NUMOPTIONS,int YBEGINLIST)
{
	for(int i =0; i < NUMOPTIONS; i++){
		if(menupos == i){
		
			ST7735_DrawStringBG(0,YBEGINLIST+i,menu_Choice[i],ST7735_WHITE,ST7735_BLUE);
		}
		else{
			ST7735_DrawStringBG(0,YBEGINLIST+i,menu_Choice[i],ST7735_BLACK,ST7735_WHITE);
		}
	}
}




