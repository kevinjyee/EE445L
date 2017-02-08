#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "fixed.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFO.h"
#include "Clock.h"
#include "Lab3.h"

#define PA3						(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2           (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1           (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0					  (*((volatile uint32_t *)0x40004004)) // Down switch


#define TRUE 1 
#define FALSE 0

#define HOUR 0
#define MIN 1 
#define SEC 2
#define MER 3
#define ALL 4

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void setTime(void);


int selectSwitchToggled = FALSE;
int redrawTime = TRUE;
bool updateTime = true;
uint32_t time;
volatile int curentMenuPos =0;
volatile int currentSetTimePos = 0 ; //-1 just entered menu	//0 is hours, 1 is minutes, 2 is seconds, 3 is Meridian


uint32_t MainScreen(uint32_t);
uint32_t SetTime(uint32_t);
uint32_t SetAlarm(uint32_t);


void set_Time(int hours, int minutes, int seconds, int meridian){
	DisableInterrupts();
	SysTick_Set_Time(hours, minutes, seconds, meridian);
	EnableInterrupts();
}

uint32_t Next_State(uint32_t current_state, uint32_t keyInputs)
{
	switch(current_state){
		case 0x00:
			return MainScreen(keyInputs);
		case 0x01:
			return SetTime(keyInputs);
		case 0x02:
			return SetAlarm(keyInputs);
		default:
			return MainScreen(keyInputs);
	}
}



void clear_Menu(){
			ST7735_FillRect(0,0,120,2,ST7735_BLACK);
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
		
		case 0x01: 
			//down switch
			curentMenuPos = (curentMenuPos + 1)%3;
			break;
		case 0x02:
			//Up Switch
			
			curentMenuPos = (curentMenuPos - 1);
			if(curentMenuPos < 0 )
			{
				curentMenuPos = 2; 
			}
			break;
		case 0x04:
			//Select Switch
			return processMenuItem(curentMenuPos);
		case 0x08: 
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
	
	uint32_t hours,minutes,seconds,meridian;
	uint32_t modifiedtime = 0;
	if(updateTime)
	{
		time = Time;
		updateTime = false;
		
	}
	modifiedtime = time;
  char hourBuffer[2] = {' '}; //Initialize array to empty string 
	char minuteBuffer[2] = {' '};
	char secondsBuffer[2] = {' '};
	char meridianBuffer[2] = {' '};

	
	seconds = modifiedtime % 100;
	modifiedtime = modifiedtime / 1000;
	minutes = modifiedtime % 100;
	modifiedtime = modifiedtime / 1000;
	hours = modifiedtime % 100;
	meridian = (modifiedtime / 100) % 10;
	
	format_setTime(hourBuffer,HOUR);
	format_setTime(minuteBuffer, MIN);
	format_setTime(secondsBuffer, SEC);
	format_setTime(meridianBuffer,MER);
	
	if(redrawTime)
	{
		ST7735_SetCursor(25,0);
		//ST7735_OutString("Set Time");
		clear_Menu();
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		
		redrawTime = FALSE;
	}
	
	DelayWait2ms(1);

	//Determine Input and Action on Configuring Time
	switch(input)
	{
		
		case 0x01: 
			//DownSwitch
			switch(currentSetTimePos)
			{
				case 0: //Hour Field
					hours = (hours - 1)%12;
					if(hours == 0)
					{
						hours = 12;
					}
					break;
				case 1: //Minute Field
					minutes = (minutes - 1)%60;
					if(minutes == 0)
					{
						minutes = 60;
					}
					break;
				case 2:	//Seconds Field
					seconds = (seconds -1)%60;
					if(seconds == 0)
					{
						seconds = 60;
					}
					break;
				case 3: //Meridian Field
					meridian = (meridian + 1)%2;
					break;
			}
			break;
		case 0x02:
			//Up Switch
			switch(currentSetTimePos)
			{
				case 0: //Hour Field
					hours = (hours + 1)%12;
					if(hours == 0)
					{
						hours = 12;
					}
					break;
				case 1: //Minute Field
					minutes = (minutes + 1)%60;
					break;
				case 2: //Seconds Field	
					seconds = (seconds + 1)%60;
					break;
				case 3: //Meridian Field
					meridian = (meridian + 1)%2;
					break;
			}
			break;
		case 0x04:
			//Select Switch
			if(currentSetTimePos == 3)
			{
				currentSetTimePos =0;
				redrawTime = TRUE;
				updateTime = true;
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				set_Time(hours,minutes,seconds,meridian);
				return 0x00;//back to main screen
			}
			else{	
			currentSetTimePos = (currentSetTimePos +1)%4; //increment field to adjust
			}
			break;
		case 0x08: 
				currentSetTimePos =0;
				redrawTime = TRUE;
				updateTime = true;
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				set_Time(hours,minutes,seconds,meridian);
				return 0x00;//back to main screen
			return 0x00;
		
	}
		Time = (100000000 * meridian) + (1000000 * hours) + (1000 * minutes) + seconds;
		time = Time;
		format_setTime(hourBuffer,HOUR);
		format_setTime(minuteBuffer, MIN);
		format_setTime(secondsBuffer, SEC);
		format_setTime(meridianBuffer,MER);
	switch(currentSetTimePos)
	{
		
		case 0:
			
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		break;
		case 1:
				ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_CYAN);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		break;
		
		case 2:
			ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_CYAN);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		break;
		case 3:
				ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_CYAN);
		break;
	
	}

	

	
	
	
	
	return 0x01;
	
}


uint32_t SetAlarm(uint32_t input)
{
	
	uint32_t hours,minutes,seconds,meridian;
	uint32_t modifiedtime = 0;
	if(updateTime)
	{
		time = Time;
		updateTime = false;
		
	}
	modifiedtime = time;
  char hourBuffer[2] = {' '}; //Initialize array to empty string 
	char minuteBuffer[2] = {' '};
	char secondsBuffer[2] = {' '};
	char meridianBuffer[2] = {' '};

	
	seconds = modifiedtime % 100;
	modifiedtime = modifiedtime / 1000;
	minutes = modifiedtime % 100;
	modifiedtime = modifiedtime / 1000;
	hours = modifiedtime % 100;
	meridian = (modifiedtime / 100) % 10;
	
	format_setTime(hourBuffer,HOUR);
	format_setTime(minuteBuffer, MIN);
	format_setTime(secondsBuffer, SEC);
	format_setTime(meridianBuffer,MER);
	
	if(redrawTime)
	{
		ST7735_SetCursor(25,0);
		//ST7735_OutString("Set Alarm");
		clear_Menu();
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		
		redrawTime = FALSE;
	}
	
	DelayWait2ms(1);

	//Determine Input and Action on Configuring Time
	switch(input)
	{
		
		case 0x01: 
			//DownSwitch
			switch(currentSetTimePos)
			{
				case 0: //Hour Field
					hours = (hours - 1)%12;
					if(hours == 0)
					{
						hours = 12;
					}
					break;
				case 1: //Minute Field
					minutes = (minutes - 1)%60;
					if(minutes == 0)
					{
						minutes = 60;
					}
					break;
				case 2:	//Seconds Field
					seconds = (seconds -1)%60;
					if(seconds == 0)
					{
						seconds = 60;
					}
					break;
				case 3: //Meridian Field
					meridian = (meridian + 1)%2;
					break;
			}
			break;
		case 0x02:
			//Up Switch
			switch(currentSetTimePos)
			{
				case 0: //Hour Field
					hours = (hours + 1)%12;
					if(hours == 0)
					{
						hours = 12;
					}
					break;
				case 1: //Minute Field
					minutes = (minutes + 1)%60;
					break;
				case 2: //Seconds Field	
					seconds = (seconds + 1)%60;
					break;
				case 3: //Meridian Field
					meridian = (meridian + 1)%2;
					break;
			}
			break;
		case 0x04:
			//Select Switch
			if(currentSetTimePos == 3)
			{
				currentSetTimePos =0;
				redrawTime = TRUE;
				updateTime = true;
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				set_Time(hours,minutes,seconds,meridian);
				return 0x00;//back to main screen
			}
			else{	
			currentSetTimePos = (currentSetTimePos +1)%4; //increment field to adjust
			}
			break;
		case 0x08: 
				currentSetTimePos =0;
				redrawTime = TRUE;
				updateTime = true;
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				set_Time(hours,minutes,seconds,meridian);
				return 0x00;//back to main screen
			return 0x00;
		
	}
		Time = (100000000 * meridian) + (1000000 * hours) + (1000 * minutes) + seconds;
		time = Time;
		format_setTime(hourBuffer,HOUR);
		format_setTime(minuteBuffer, MIN);
		format_setTime(secondsBuffer, SEC);
		format_setTime(meridianBuffer,MER);
	switch(currentSetTimePos)
	{
		
		case 0:
			
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		break;
		case 1:
				ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_CYAN);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		break;
		
		case 2:
			ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_CYAN);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		break;
		case 3:
				ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_CYAN);
		break;
	
	}

	

	
	
	
	
	return 0x01;
	
}
