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
#include "Song.h"
#include "FSM.h"
#include "SongScreen.h"

#define XTITLE 5
#define YTITLE 0
#define TITLEBORDER 11
#define YITEMS 2


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
			//Select Switch
			//return processMenuItem(curentMenuPos);
		case MENU:
			//Menu Button
			return 0x00;
	}
	draw_SongScreen(SongName);
	
return rtSongScreen;
}