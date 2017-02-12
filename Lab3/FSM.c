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
#include "LCD.h"

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
#define NUMALARMS 8
#define NUMSONGS 5

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void setTime(void);

int selectSwitchToggled = FALSE;
int redrawTime = true;
extern volatile int redrawHands;
extern volatile bool animateAlarm;
bool updateTime = true;
uint32_t time;
volatile int curentMenuPos =0;
volatile int currentSetTimePos = 0 ; //-1 just entered menu	//0 is hours, 1 is minutes, 2 is seconds, 3 is Meridian
volatile int currentSetAlarmPos = 0;
volatile int setMultipleAlarmPos =0;
volatile int currentSongPos = 0;

uint32_t AlarmTimeArray[NUMALARMS] ={0};
int AlarmONOFFArray[NUMALARMS] = {0};
int AlarmOn = FALSE;

uint32_t MainScreen(uint32_t);
uint32_t SetTime(uint32_t);
uint32_t SetAlarm(uint32_t);
uint32_t SetAlarms(uint32_t);
uint32_t SetMultipleAlarm(uint32_t);
uint32_t ChooseSong(uint32_t);

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
			return SetMultipleAlarm(keyInputs);
		case 0x03:
			return ChooseSong(keyInputs);
		case 0x04:
			return SetAlarms(keyInputs);
		default:
			return MainScreen(keyInputs);
	}
}


void clear_Menu(){
			ST7735_FillRect(0,0,125,10,ST7735_BLACK);
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
	
	int hours,minutes,seconds,meridian;
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
	
	format_setTime(hourBuffer,HOUR,FALSE,FALSE);
	format_setTime(minuteBuffer, MIN,FALSE,FALSE);
	format_setTime(secondsBuffer, SEC,FALSE,FALSE);
	format_setTime(meridianBuffer,MER,FALSE,FALSE);
	
	if(redrawTime)
	{
		
		clear_Menu();
		ST7735_DrawString(0,0,"Set Time",ST7735_WHITE);
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		
		redrawTime = FALSE;
	}
	
	

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
					if(minutes <= 0)
					{
						minutes = 59;
					}
					break;
				case 2:	//Seconds Field
					seconds = (seconds -1)%60;
					if(seconds <= 0)
					{
						seconds = 59;
					}
					break;
				case 3: //Meridian Field
					meridian = (meridian + 1)%2;
					break;
			}
			draw_Hands(minutes, hours);
			break;
		case 0x02:
			//Up Switch
			switch(currentSetTimePos)
			{
				case 0: //Hour Field
					hours = (hours + 1)%12;
					if(hours <= 0)
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
			draw_Hands(minutes, hours);
			break;
		case 0x04:
			//Select Switch
			if(currentSetTimePos == 3)
			{
				currentSetTimePos =0;
				redrawTime = TRUE;
				redrawHands = TRUE;
				updateTime = true;
				ST7735_DrawString(0,0,"Set Time",ST7735_BLACK);
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				clear_OldHands();
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
				redrawHands = TRUE;
				ST7735_DrawString(0,0,"Set Time",ST7735_BLACK);
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
		/*
				updateTime = true;
				ST7735_DrawString(0,0,"Set Time",ST7735_BLACK);
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				
				clear_OldHands();
		*/
				//set_Time(hours,minutes,seconds,meridian);
				
				return 0x00;//back to main screen
			return 0x00;
		
	}
		Time = (100000000 * meridian) + (1000000 * hours) + (1000 * minutes) + seconds;
		time = Time;
		format_setTime(hourBuffer,HOUR,FALSE,FALSE);
		format_setTime(minuteBuffer, MIN,FALSE,FALSE);
		format_setTime(secondsBuffer, SEC,FALSE,FALSE);
		format_setTime(meridianBuffer,MER,FALSE,FALSE);
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

void formatAlarmType(char* buffer,int i )
{
	char alarmchoice = i + '0';
	buffer[0]='A';
	buffer[1]='l';
	buffer[2]='a';
	buffer[3]='r';
	buffer[4]='m';
	buffer[5]=' ';
	buffer[6]= alarmchoice;
}
uint32_t SetMultipleAlarm(uint32_t input)
{
	if(redrawTime)
	{
		ST7735_FillScreen(0);
		redrawTime = FALSE;
	}
	
	switch(input)
	{
			case 0x01: 
			//down switch
			setMultipleAlarmPos = (setMultipleAlarmPos + 1)%NUMALARMS;
			break;
		case 0x02:
			//Up Switch
			
			setMultipleAlarmPos = (setMultipleAlarmPos - 1)%NUMALARMS;
			if(setMultipleAlarmPos < 0 )
			{
				setMultipleAlarmPos = NUMALARMS - 1; 
			}
			break;
		case 0x04:
			//Select Switch
			redrawTime = true;
			redrawHands = TRUE;
			input = 0x00;
			return SetAlarms(input);
		case 0x08: 
			//Menu Button
			redrawTime = true;
			redrawHands = TRUE;
			draw_Clock();
			draw_Time();
			return 0x00;
		
	}
		for(int i =0; i < NUMALARMS; i++)
	{
		char buffer[7] ={' '};
		formatAlarmType(buffer,i);
		if(setMultipleAlarmPos == i)
		{
			ST7735_DrawString(0,i,buffer,ST7735_CYAN);
		}
		else{
			ST7735_DrawString(0,i,buffer,ST7735_WHITE);
		}
		
	}

	return 0x02;
}

//Test Function for Multiple Set Alarms
uint32_t SetAlarms(uint32_t input)
{
	
	int hours,minutes,seconds,meridian;
	uint32_t modifiedtime = 0;

	time = AlarmTimeArray[setMultipleAlarmPos];
		
	
	
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
	
	if(hours == 0)
	{
			hours = 12;
	}
	format_setTime(hourBuffer,HOUR,TRUE,setMultipleAlarmPos);
	format_setTime(minuteBuffer, MIN,TRUE,setMultipleAlarmPos);
	format_setTime(secondsBuffer, SEC,TRUE,setMultipleAlarmPos);
	format_setTime(meridianBuffer,MER,TRUE,setMultipleAlarmPos);
	
	if(redrawTime)
	{
		
		clear_Menu();
		ST7735_FillScreen(0);
		ST7735_DrawString(0,0,"Set Alarm",ST7735_WHITE);
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		ST7735_DrawString(11,1," ",ST7735_WHITE);
		ST7735_DrawString(12,1,"OFF",ST7735_WHITE);
		
		redrawTime = FALSE;
	}
	
	

	//Determine Input and Action on Configuring Time
	switch(input)
	{
		
		case 0x01: 
			//DownSwitch
			switch(currentSetAlarmPos)
			{
				case 0: //Hour Field
					hours = (hours - 1)%12;
					if(hours <= 0)
					{
						hours = 12;
					}
					break;
				case 1: //Minute Field
					minutes = (minutes - 1)%60;
					if(minutes <= 0)
					{
						minutes = 59;
					}
					break;
				case 2:	//Seconds Field
					seconds = (seconds -1)%60;
					if(seconds <= 0)
					{
						seconds = 59;
					}
					break;
				case 3: //Meridian Field
					meridian = (meridian + 1)%2;
					break;
				case 4: //OFF or ON
					AlarmONOFFArray[setMultipleAlarmPos] = (AlarmONOFFArray[setMultipleAlarmPos]+1)%2;
					break;
			}
			break;
		case 0x02:
			//Up Switch
			switch(currentSetAlarmPos)
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
				case 4: //
					AlarmONOFFArray[setMultipleAlarmPos] = (AlarmONOFFArray[setMultipleAlarmPos] +1)%2;
					break;
			}
			break;
		case 0x04:
			//Select Switch
			if(currentSetAlarmPos == 4)
			{
				currentSetAlarmPos =0;
				redrawTime = TRUE;
				redrawHands = TRUE;
				updateTime = true;
				ST7735_DrawString(0,0,"Set Alarm",ST7735_BLACK);
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_BLACK);

				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_BLACK);
			
				}
				AlarmTimeArray[setMultipleAlarmPos] = (100000000 * meridian) + (1000000 * hours) + (1000 * minutes) + seconds;
				return 0x02;//back to select screen
			}
			else{	
			currentSetAlarmPos = (currentSetAlarmPos +1)%5; //increment field to adjust
			}
			break;
			case 0x08: 
				currentSetAlarmPos = 0;
				redrawTime = TRUE;
				redrawHands = TRUE;
				updateTime = true;
				ST7735_DrawString(0,0,"Set Alarm",ST7735_BLACK);
				ST7735_DrawString(0,1,hourBuffer,ST7735_BLACK);
				ST7735_DrawString(2,1,":",ST7735_WHITE);
				ST7735_DrawString(3,1,minuteBuffer,ST7735_BLACK);
				ST7735_DrawString(5,1,":",ST7735_WHITE);
				ST7735_DrawString(6,1,secondsBuffer,ST7735_BLACK);
				ST7735_DrawString(8,1," ",ST7735_WHITE);
				ST7735_DrawString(9,1,meridianBuffer,ST7735_BLACK);
				ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_BLACK);
					
				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_BLACK);
					
				}
				AlarmTimeArray[setMultipleAlarmPos] = (100000000 * meridian) + (1000000 * hours) + (1000 * minutes) + seconds;
				draw_Clock();
				return 0x00;//back to main screen
			return 0x00;
		
	}
		AlarmTimeArray[setMultipleAlarmPos] = (100000000 * meridian) + (1000000 * hours) + (1000 * minutes) + seconds;
	
		format_setTime(hourBuffer,HOUR,TRUE,setMultipleAlarmPos);
		format_setTime(minuteBuffer, MIN,TRUE,setMultipleAlarmPos);
		format_setTime(secondsBuffer, SEC,TRUE,setMultipleAlarmPos);
		format_setTime(meridianBuffer,MER,TRUE,setMultipleAlarmPos);
	switch(currentSetAlarmPos)
	{
		
		case 0:
			
		ST7735_DrawString(0,1,hourBuffer,ST7735_CYAN);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_WHITE);
				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_WHITE);
				}
		break;
		case 1:
				ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_CYAN);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
		ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_WHITE);
				
				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_WHITE);
				}
		break;
		
		case 2:
			ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_CYAN);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
			ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_WHITE);
				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_WHITE);
				}
		break;
		case 3:
				ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_CYAN);
		ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_WHITE);
				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_WHITE);
				}
		break;
		case 4:
		ST7735_DrawString(0,1,hourBuffer,ST7735_WHITE);
		ST7735_DrawString(2,1,":",ST7735_WHITE);
		ST7735_DrawString(3,1,minuteBuffer,ST7735_WHITE);
		ST7735_DrawString(5,1,":",ST7735_WHITE);
		ST7735_DrawString(6,1,secondsBuffer,ST7735_WHITE);
		ST7735_DrawString(8,1," ",ST7735_WHITE);
		ST7735_DrawString(9,1,meridianBuffer,ST7735_WHITE);
			ST7735_DrawString(11,1," ",ST7735_WHITE);
				if(AlarmONOFFArray[setMultipleAlarmPos]){
					ST7735_DrawString(12,1,"ON ",ST7735_CYAN);
				}
				else{
				ST7735_DrawString(12,1,"OFF",ST7735_CYAN);
				}
		break;
	
	}

	return 0x04;
	
}

void formatSongType(char* buffer,int i )
{
	char songchoice = i + '0';
	buffer[0]='S';
	buffer[1]='o';
	buffer[2]='n';
	buffer[3]='g';
	buffer[4]=' ';
	buffer[5]=songchoice;
}

uint32_t ChooseSong(uint32_t input)
{
if(redrawTime)
	{
		ST7735_FillScreen(0);
		redrawTime = FALSE;
	}
	
	switch(input)
	{
			case 0x01: 
			//down switch
			currentSongPos = (currentSongPos + 1)%NUMSONGS;
			break;
		case 0x02:
			//Up Switch
			
			currentSongPos = (currentSongPos - 1)%NUMSONGS;
			if(currentSongPos < 0 )
			{
				currentSongPos = NUMSONGS-1; 
			}
			break;
		case 0x04:
			//Select Switch
		  draw_Clock();
			draw_Time();
			redrawTime = true;
			redrawHands = TRUE;
			draw_Time();
			return 0x00;
		case 0x08: 
			//Menu Button
			redrawTime = true;
			redrawHands = TRUE;
			draw_Clock();
			draw_Time();
			return 0x00;
		
	}
	
	for(int i =0; i < NUMSONGS; i++)
	{
		char buffer[6] = {'S','o','n','g',' ','1'};
		formatSongType(buffer,i);
		if(i == currentSongPos)
		{
			ST7735_DrawString(0,i,buffer,ST7735_CYAN);
		}
		else{
			ST7735_DrawString(0,i,buffer,ST7735_WHITE);
		}
	}
		

	return 0x03;
	
	
}
