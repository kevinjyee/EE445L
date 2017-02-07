#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "fixed.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFO.h"


#define PA3							(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2             (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1             (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0							(*((volatile uint32_t *)0x40004004)) // Down switch


#define TRUE 1 
#define FALSE 0


int selectSwitchToggled = FALSE;

volatile int curentMenuPos =0;
volatile int currentTimePos = 0;

uint32_t MainScreen(uint32_t);
uint32_t SetTime(uint32_t);




uint32_t Next_State(uint32_t current_state, uint32_t keyInputs)
{
	switch(current_state){
		case 0x00:
			return MainScreen(keyInputs);
		case 0x01:
			return SetTime(keyInputs);
		default:
			return MainScreen(keyInputs);
	}
}


void clear_Menu(){
			ST7735_DrawString(0,1,"Set Time",ST7735_BLACK);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_BLACK);
			ST7735_DrawString(0,3,"Choose Song",ST7735_BLACK);
	
}


uint32_t processMenuItem(int currentMenuPos)
{
	int x;
		switch(currentMenuPos)
		{
			case 0: 
				//Set Time 
				return 0x01;
			case 1: 
				//Set Alarm
				return 0x02;
			case 2:
				//Choose Song
				return 0x03;
		}
		return 0x00;
}



uint32_t MainScreen(uint32_t input)
{
	
	//Determine Input and Action on MainScreen
	switch(input)
	{
		
		case 1: 
			//down switch
			curentMenuPos = (curentMenuPos + input)%3;
			break;
		case 2:
			//Up Switch
			curentMenuPos = (curentMenuPos - input)%3;
			break;
		case 3:
			//Select Switch
			return processMenuItem(curentMenuPos);
		case 4: 
			//Menu Button
			return 0x00;
		
	}
	//Determine which Menu Position I'm in
	switch(curentMenuPos)
	{
		case 0:
			ST7735_DrawString(0,1,"Set Time",ST7735_CYAN);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_WHITE);
			ST7735_DrawString(0,3,"Choose Song",ST7735_WHITE);
			break;
		case 1:
			ST7735_DrawString(0,1,"Set Time",ST7735_WHITE);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_CYAN);
			ST7735_DrawString(0,3,"Choose Song",ST7735_WHITE);
			break;
		case 2:
			ST7735_DrawString(0,1,"Set Time",ST7735_WHITE);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_WHITE);
			ST7735_DrawString(0,3,"Choose Song",ST7735_CYAN);
			break;
	}
	return 0x00;
}

	
uint32_t SetTime(uint32_t input)
{
	clear_Menu();
	
	uint32_t time = Time;
	
	char hourBuffer[2] = {' '}; //Initialize array to empty string 
	char minuteBuffer[2] = {' '};
	char secondsBuffer[2] = {' '};
	
	//Determine Input and Action on MainScreen
	switch(input)
	{
		
		case 1: 
			//down switch
			curentMenuPos = (curentMenuPos + input)%3;
			break;
		case 2:
			//Up Switch
			curentMenuPos = (curentMenuPos - input)%3;
			break;
		case 3:
			//Select Switch
			return processMenuItem(curentMenuPos);
		case 4: 
			//Menu Button
			return 0x00;
		
	}
	//Determine which Menu Position I'm in
	switch(curentMenuPos)
	{
		case 0:
			ST7735_DrawString(0,1,"Set Time",ST7735_CYAN);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_WHITE);
			ST7735_DrawString(0,3,"Choose Song",ST7735_WHITE);
			break;
		case 1:
			ST7735_DrawString(0,1,"Set Time",ST7735_WHITE);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_CYAN);
			ST7735_DrawString(0,3,"Choose Song",ST7735_WHITE);
			break;
		case 2:
			ST7735_DrawString(0,1,"Set Time",ST7735_WHITE);
			ST7735_DrawString(0,2,"Set Alarm",ST7735_WHITE);
			ST7735_DrawString(0,3,"Choose Song",ST7735_CYAN);
			break;
	}

	
	
	
	
	return 0;
	
}

