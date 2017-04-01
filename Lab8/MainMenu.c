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



static int8_t NUMMENUITEMS = 3;
static int8_t  MenuPos = 0;
////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160


char Songs[18] = {' ', ' ', 'S', 'o', 'n', 'g', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>', 0};
char Health[18] = {' ', ' ', 'H', 'e', 'a', 'l', 't', 'h', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>',0};
char Settings[18] = {' ', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' ', ' ', ' ', ' ', ' ', ' ', '>',0};

char* menu_Choice[3] = {		
	Songs, Health, Settings
};

uint32_t processMenuItem(int menupos)
{
	switch(menupos)
	{
		case 0:
			return rtSongMenu;
		case 1:
			return rtPedometer;
		case 2:
			break;
			//Settings
	}
	return 0x00;
}

uint32_t MainScreen(uint32_t input){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Menu");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case DOWN: 
			//down switch
			MenuPos = (MenuPos + 1)%NUMMENUITEMS;
			break;
		case UP:
			//Up Switch
			MenuPos = (MenuPos - 1);
			if(MenuPos < 0 )
			{
				MenuPos = NUMMENUITEMS-1; 
			}
			break;
		case LEFT:
			//Do Nothing
			break;
		case RIGHT:
			//Do Nothing
			break;
		case MENU:
			//Do Nothing
			break;
		case SELECT:
			return processMenuItem(MenuPos);
	}
	Draw_Options(MenuPos,menu_Choice,NUMMENUITEMS,YITEMS);
	return rtMainScreen;
}