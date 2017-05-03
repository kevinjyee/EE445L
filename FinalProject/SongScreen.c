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
#define XITEMS 2

#define TOGGLE_PLAY 0x40
#define CHANGE_SONG 0x80
#define REWIND 			0xF0
#define PLAY 				1
#define PAUSE 			0
#define MAXVOLUME 7
#define DISPLAY_LENGTH 120


extern char Play_Toggled;
//volatile int lastSongPos = -1;
bool firstEntrance = true;
bool musicplaying = false;
volatile int Volume = 3;
volatile uint8_t Title_Display_Index = 0;
char oldTitle[DISPLAY_LENGTH];


////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160

void draw_SongScreen(char* songname){
	/*
	char display[DISPLAY_LENGTH];
	for(int i = 0; i < DISPLAY_LENGTH; i++){
		//if(i < SONG_TITLE_LENGTH){
			
			*(display + ((Title_Display_Index + i) % DISPLAY_LENGTH)) = *(songname + (i  % SONG_TITLE_LENGTH));
		//} else{
		//	*(display + i) = ' ';
		//}
	}
	Title_Display_Index = (Title_Display_Index + 1) % DISPLAY_LENGTH;
	//ST7735_DrawString(0,YITEMS,oldTitle,ST7735_WHITE);
	//ST7735_DrawStringBG(0,YITEMS,oldTitle,ST7735_WHITE, ST7735_WHITE);
	for(int i = 0; i < DISPLAY_LENGTH; i++){
		oldTitle[i] = display[i];
	}
	*/
	ST7735_DrawString(0,YITEMS,songname,ST7735_BLACK);
	
}

void draw_Graphics(){
	ST7735_DrawString(XTITLE+1,YITEMS+1,"-",ST7735_BLACK);
	for(int i =1; i <= MAXVOLUME; i ++)
	{
		if(i<Volume)
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
	if(!Play_Toggled)
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
		if(Volume <= 0)
		{
			Volume =0;
		}
		Volume -=1;
			
			break;
		case UP:
			//Up Switch
			//Volume Down
			if(Volume >=7)
			{
				Volume = 7;
			}
			Volume +=1;
			break;
		case SELECT:
			if(firstEntrance)
			{
				firstEntrance= false;
			}
			else{
			Play_Toggled ^= PLAY;
			if(Play_Toggled){
				//Pause();
				if(Current_Song != Song_Menu_Pos){
					Change_Song();
					Current_Song = Song_Menu_Pos;
					ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
					break;
				} else{
					Play();
					Current_Song = Song_Menu_Pos;
					ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
				}
			} else{
				Pause();
				ST7735_DrawBitmap(55,135,playbutton24bit,24,24);
			}
			
			/*
			if((Current_Song != Song_Menu_Pos) && (Play_Toggled)){
				Change_Song();
			//	musicplaying = true;
				ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
				break;
			} else{
				if(Play_Toggled){
					//Current_Song = Song_Menu_Pos;
					Play();
				//	musicplaying = true;
					ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
				}
				else{
					Pause();
					Current_Song = Song_Menu_Pos;
				//	musicplaying = false;
					ST7735_DrawBitmap(55,135,playbutton24bit,24,24);
				}
			}
			*/
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
			Song_Menu_Pos = (Song_Menu_Pos + 1)%NUM_SONGS;
			if(Play_Toggled){
				Pause();
			}
			Change_Song();
			ST7735_DrawBitmap(55,135,pausebutton24bit,24,24);
			Play_Toggled |= PLAY;
			return rtSongScreen;
	}
	draw_SongScreen(SongName);
	draw_Graphics();
return rtSongScreen;
}