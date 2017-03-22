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

#define PA3						(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2           (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1           (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0					  (*((volatile uint32_t *)0x40004004)) // Down switch
	
#define XTITLE 5
#define YTITLE 0
#define TITLEBORDER 11
#define YITEMS 2

uint8_t NUMMENUITEMS = 3;
uint8_t  SongMenuPos = 0;
////  screen is actually 129 by 161 pixels, x 0 to 128, y goes from 0 to 160


char Track1[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>', 0};
char Track2[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '2', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ','>',0};
char Track3[18] = {' ', ' ', 'S', 'o', 'n', 'g', ' ', '3', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '>',0};

char* menu_Choice[3] = {		
	Track1, Track2, Track3
};

uint32_t SongMenu(uint32_t input){
	draw_Title(XTITLE,YTITLE,TITLEBORDER,"Song Select");
	//Determine Input and Action on Song Screen
	switch(input)
	{
		case 0x01: 
			//down switch
			SongMenuPos = (SongMenuPos + 1)%NUMMENUITEMS;
			break;
		case 0x02:
			//Up Switch
			SongMenuPos = (SongMenuPos - 1);
			if(MainMenuPos < 0 )
			{
				SongMenuPos = NUMMENUITEMS-1; 
			}
			break;
		case 0x04:
			//Select Switch
			//return processMenuItem(curentMenuPos);
		case 0x08: 
			//Menu Button
			return 0x00;
	}
	draw_Options(SongMenuPos,menu_Choice,NUMMENUITEMS,YITEMS);
	return 0;
}