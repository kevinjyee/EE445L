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
#include "Globals.h"
#include "Song.h"
#include "FSM.h"
#include "SongScreen.h"
#include "Music.h"

#define XTITLE 5
#define YTITLE 0
#define TITLEBORDER 11
#define YITEMS 2

#define TOGGLE_PLAY 0x40
#define CHANGE_SONG 0x80
#define REWIND 			0xF0
#define PLAY 				1
#define PAUSE 			0
extern char Play_Toggled;
volatile int lastSongPos = -1;
bool firstEntrance = true;

void draw_SongScreen(char* songname){
	ST7735_DrawString(XTITLE,YITEMS,songname,ST7735_BLACK);
	
}
uint32_t SongScreen(uint32_t input,char* SongName, const uint16_t* Graphic){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Now Playing");
	//Determine Input and Action on Song Screen
	switch(input)
	{
		case DOWN:
			//down switch Volume Up
			break;
		case UP:
			//Up Switch
			//Volume Down
			
			break;
		case SELECT:
			if(firstEntrance)
			{
				firstEntrance= false;
				
			}
			else{
			Play_Toggled ^= PLAY;
			if((currentSongPos != lastSongPos) && (Play_Toggled == PLAY)){
				lastSongPos = currentSongPos;
				Change_Song();
				break;
			} else{
				if(Play_Toggled){
					Play();
				}
				else{
					Pause();
				}
			}
		}
		
		break;
		case LEFT:
			firstEntrance =true;
			ST7735_FillScreen(ST7735_WHITE);
			return 0x00;
			//Menu Button
			break;
	}
	draw_SongScreen(SongName);
	
return rtSongScreen;
}