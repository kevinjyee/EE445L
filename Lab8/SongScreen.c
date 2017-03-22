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

#define XTITLE 5
#define YTITLE 0
#define TITLEBORDER 11
#define YITEMS 2


void draw_SongScreen(){
	ST7735_DrawString(XTITLE,YITEMS,"*",ST7735_BLACK);
	
}
uint32_t SongScreen(uint32_t input){
	draw_Title(XTITLE,YTITLE,TITLEBORDER,"Now Playing");
	//Determine Input and Action on Song Screen
	switch(input)
	{
		case 0x01: 
			//down switch Volume Up
			break;
		case 0x02:
			//Up Switch
			//Volume Down
			
			break;
		case 0x04:
			//Select Switch
			//return processMenuItem(curentMenuPos);
		case 0x08: 
			//Menu Button
			return 0x00;
	}
	
return 0;
}