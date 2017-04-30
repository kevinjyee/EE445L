#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOQueue.h"
#include "LCD.h"
#include "Globals.h"
#include "Song.h"
#include "FSM.h"
#include "SongScreen.h"
#include "SongMenu.h"
#include "Graphics.h"
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
#define MAXVOLUME 7


extern char Play_Toggled;
volatile int lastSongPos = -1;
bool firstEntrance = true;
bool musicplaying = false;
volatile int songVolume = 3;


////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160

void draw_SongScreen(char* songname){
	ST7735_DrawString(XTITLE,YITEMS,songname,ST7735_BLACK);
	
}

void draw_Graphics(){
	ST7735_DrawString(XTITLE+1,YITEMS+1,"-",ST7735_BLACK);
	for(int i =1; i <= MAXVOLUME; i ++)
	{
		if(i<songVolume)
		{
			ST7735_DrawString(XTITLE+1+i,YITEMS+1,"_",ST7735_BLACK);
		}
		else
		{
			ST7735_DrawStringBG(XTITLE+1+i,YITEMS+1,"_",ST7735_WHITE, ST7735_WHITE);
		}
	}
	ST7735_DrawString(XTITLE+1+MAXVOLUME + 1,YITEMS+1,"+",ST7735_BLACK);
	
	ST7735_DrawBitmap(10,135,rewindbutton24bit,24,24);
	if(!musicplaying)
	{
	ST7735_DrawBitmap(55,135,playbutton24bit,24,24);
	}
	else{
		ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
	}
	ST7735_DrawBitmap(100,135,forwardbutton24bit,24,24);
}
uint32_t SongScreen(uint32_t input,char* SongName, const uint16_t* Graphic){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Now Playing");
	//Determine Input and Action on Song Screen
	switch(input)
	{
		case DOWN:
			//down switch Volume Up
		if(songVolume <= 0)
		{
			songVolume =0;
		}
		songVolume -=1;
			
			break;
		case UP:
			//Up Switch
			//Volume Down
			if(songVolume >=7)
			{
				songVolume = 7;
			}
			songVolume +=1;
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
				musicplaying = true;
				ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
				break;
			} else{
				if(Play_Toggled){
					Play();
					lastSongPos = currentSongPos;
					musicplaying = true;
					ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
				}
				else{
					Pause();
					lastSongPos = currentSongPos;
					musicplaying = false;
					ST7735_DrawBitmap(55,135,playbutton24bit,24,24);
				}
			}
		}
		
		break;
		case LEFT:
			firstEntrance =true;
			ST7735_FillScreen(ST7735_WHITE);
			return rtSongMenu;
			//Menu Button
			break;
		case RIGHT:
			ST7735_FillScreen(ST7735_WHITE);
			currentSongPos = (currentSongPos + 1)%NUMSONGS;
			Play_Toggled ^= PLAY;
			return rtSongScreen;
	}
	draw_SongScreen(SongName);
	draw_Graphics();
return rtSongScreen;
}