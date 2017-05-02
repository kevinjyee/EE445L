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


char Track1[21] = {' ', ' ', 'L', 'o', 's', 't', ' ', 'W', 'o', 'o', 'd', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '>', 0};
char Track2[21] = {' ', ' ', 'N', 'e', 'w', ' ', 'B', 'a', 'r', 'k', ' ', 'T', 'o', 'w', 'n', ' ', ' ', ' ', ' ', '>',0};
char Track3[21] = {' ', ' ', 'C', 'a', 'r', 'o', 'l', ' ', 'o', 'f', ' ', 't', 'h', 'e', 'B', 'e', 'l' , 'l', 's', '>',0};
char Track4[21] = {' ', ' ', 'B', 'a', 'c', 'h', ' ', 'C', 'e', 'l', 'l', 'o', ' ', 'S', 'u', 'i', 't' , 'e', '1', '>',0};

static const uint16_t* dummy = {0x00};

char* menu_Choices[NUM_SONGS] = {		
	Track1, Track2, Track3, Track4
};

extern SongChoice SongsList[NUM_SONGS];

uint32_t SongMenu(uint32_t input){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Song Select");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case DOWN: 
			//down switch
			currentSongPos = (currentSongPos + 1)%NUM_SONGS;
			break;
		case UP:
			//Up Switch
			currentSongPos = (currentSongPos - 1);
			if(currentSongPos < 0 )
			{
				currentSongPos = NUM_SONGS - 1; 
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
	Draw_Options(currentSongPos,menu_Choices,NUM_SONGS,YITEMS);
	return rtSongMenu;
}

void SongMenu_Init(){
	for(int i =0; i < NUM_SONGS; i++)
	{
		SongsList[i].ID=i;
		SongsList[i].SongName=menu_Choices[i];
	//initialize as wel
		
		
	}
	
}