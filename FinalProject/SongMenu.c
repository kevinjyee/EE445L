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
#include "FSM.h"
#include "SongMenu.h"
#include "SongScreen.h"
#include "Music.h"
	

////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160


char Track1[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>', 0};
char Track2[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '2', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>',0};
char Track3[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '3', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '>',0};

static const uint16_t* dummy = {0x00};

char* menu_Choices[3] = {		
	Track1, Track2, Track3
};

extern SongChoice SongsList[NUMSONGS];

uint32_t SongMenu(uint32_t input){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Song Select");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case DOWN: 
			//down switch
			currentSongPos = (currentSongPos + 1)%NUMSONGS;
			break;
		case UP:
			//Up Switch
			currentSongPos = (currentSongPos - 1);
			if(currentSongPos < 0 )
			{
				currentSongPos = NUMSONGS-1; 
			}
			break;
		//case LEFT:
			//Do Nothing
			//break;
		case RIGHT:
			//Do Nothing
			break;
		case LEFT:
			ST7735_FillScreen(ST7735_WHITE);
			return 0x00; //return back to the main screen
		case SELECT:
			ST7735_FillScreen(ST7735_WHITE);
			return SongScreen(input, SongsList[currentSongPos].SongName,dummy);
			break;
	}
	Draw_Options(currentSongPos,menu_Choices,NUMSONGS,YITEMS);
	return rtSongMenu;
}

void SongMenu_Init(){
	for(int i =0; i < NUMSONGS; i++)
	{
		SongsList[i].ID=i;
		SongsList[i].SongName=menu_Choices[i];
	//initialize as wel
		
		
	}
	
}