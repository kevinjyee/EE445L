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


#define XTITLE 5
#define YTITLE 0
#define TITLEBORDER 11
#define YITEMS 2

uint8_t NUMMENUITEMS = 3;
uint8_t  MainMenuPos = 0;
////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160


char Songs[18] = {' ', ' ', 'S', 'o', 'n', 'g', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>', 0};
char Health[18] = {' ', ' ', 'H', 'e', 'a', 'l', 't', 'h', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>',0};
char Settings[18] = {' ', ' ', 'S', 'e', 't', 't', 'i', 'n', 'g', 's', ' ', ' ', ' ', ' ', ' ', ' ', '>',0};

char* menu_Choice[3] = {		
	Songs, Health, Settings
};

uint32_t MainScreen(uint32_t input){
	draw_Title(XTITLE,YTITLE,TITLEBORDER,"Menu");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case 0x01: 
			//down switch
			MainMenuPos = (MainMenuPos + 1)%NUMMENUITEMS;
			break;
		case 0x02:
			//Up Switch
			MainMenuPos = (MainMenuPos - 1);
			if(MainMenuPos < 0 )
			{
				MainMenuPos = NUMMENUITEMS-1; 
			}
			break;
		case 0x04:
			//Select Switch
			//return processMenuItem(curentMenuPos);
		case 0x08: 
			//Menu Button
			return 0x00;
	}
	draw_Options(MainMenuPos,menu_Choice,NUMMENUITEMS,YITEMS);
	return 0;
}