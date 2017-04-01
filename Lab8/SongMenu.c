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
#include "Globals.h"
#include "FSM.h"
#include "SongScreen.h"

	

volatile int8_t  SongMenuPos = 0;
////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160


char Track1[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>', 0};
char Track2[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '2', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>',0};
char Track3[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '3', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '>',0};

	typedef struct
{
	int ID;
	char* SongName;
	const uint16_t* Graphic;
} SongChoice;


#define NUMSONGS 3
	
SongChoice SongsList[NUMSONGS];
const uint16_t* dummy = {0x00};
char* menu_Choices[3] = {		
	Track1, Track2, Track3
};

uint32_t SongMenu(uint32_t input){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Song Select");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case DOWN: 
			//down switch
			SongMenuPos = (SongMenuPos + 1)%NUMSONGS;
			break;
		case UP:
			//Up Switch
			SongMenuPos = (SongMenuPos - 1);
			if(SongMenuPos < 0 )
			{
				SongMenuPos = NUMSONGS-1; 
			}
			break;
		case LEFT:
			//Do Nothing
			break;
		case RIGHT:
			//Do Nothing
			break;
		case MENU:
			return 0x00; //return back to the main screen
		case SELECT:
			return SongScreen(input, SongsList[SongMenuPos].SongName,dummy);
			break;
	}
	Draw_Options(SongMenuPos,menu_Choices,NUMSONGS,YITEMS);
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