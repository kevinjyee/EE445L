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


char paddedTrack1[SONG_TITLE_LENGTH] = { ' ', ' ', ' ',' ', ' ', ' ', 'L', 'o', 's', 't', ' ', 'W', 'o', 'o', 'd', 's', ' ', ' ', ' ',' ', ' ', ' ', 0};
char paddedTrack2[SONG_TITLE_LENGTH] = {' ', ' ', ' ',' ', 'N', 'e', 'w', ' ', 'B', 'a', 'r', 'k', ' ', 'T', 'o', 'w', 'n', ' ', ' ', ' ',' ', 0};
char paddedTrack3[SONG_TITLE_LENGTH] = {' ', ' ', 'C', 'a', 'r', 'o', 'l', ' ', 'o', 'f', ' ', 't', 'h', 'e', ' ', 'B', 'e', 'l' , 'l', 's', ' ', 0};
char paddedTrack4[SONG_TITLE_LENGTH] = {'B', 'a', 'c', 'h', ' ', 'C', 'e', 'l', 'l', 'o', ' ', 'S', 'u', 'i', 't' , 'e', ' ', 'N', 'o', ' ', '1',0};

char Track1[SONG_TITLE_LENGTH] = {' ', ' ', 'L', 'o', 's', 't', ' ', 'W', 'o', 'o', 'd', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '>', 0};	
char Track2[SONG_TITLE_LENGTH] = {' ', ' ', 'N', 'e', 'w', ' ', 'B', 'a', 'r', 'k', ' ', 'T', 'o', 'w', 'n', ' ', ' ', ' ', ' ', ' ', '>', 0};
char Track3[SONG_TITLE_LENGTH] = {' ', ' ', 'C', 'a', 'r', 'o', 'l', ' ', 'o', 'f', ' ', 't', 'h', 'e', ' ', 'B', 'e', 'l' , 'l', 's', '>', 0};
char Track4[SONG_TITLE_LENGTH] = {' ', ' ', 'B', 'a', 'c', 'h', ' ', 'C', 'e', 'l', 'l', 'o', ' ', 'S', 'u', 'i', 't' , 'e', ' ', ' ', '>', 0};

static const uint16_t* dummy = {0x00};

char* menu_Choices[NUM_SONGS] = {		
	Track1, Track2, Track3, Track4
};

char* paddedMenuChoices[NUM_SONGS] = {
	paddedTrack1, paddedTrack2, paddedTrack3, paddedTrack4
};

extern SongChoice SongsList[NUM_SONGS];

uint32_t SongMenu(uint32_t input){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Song Select");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case DOWN: 
			//down switch
			Song_Menu_Pos = (Song_Menu_Pos + 1)%NUM_SONGS;
			break;
		case UP:
			//Up Switch
			Song_Menu_Pos = (Song_Menu_Pos - 1);
			if(Song_Menu_Pos < 0 )
			{
				Song_Menu_Pos = NUM_SONGS - 1; 
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
			return SongScreen(input, SongsList[Song_Menu_Pos].SongName,dummy);
			break;
	}
	Draw_Options(Song_Menu_Pos,menu_Choices,NUM_SONGS,YITEMS);
	return rtSongMenu;
}

void SongMenu_Init(){
	for(int i =0; i < NUM_SONGS; i++)
	{
		SongsList[i].ID=i;
		SongsList[i].SongName = paddedMenuChoices[i];
	//initialize as wel
		
		
	}
	
}