#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "fixed.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFO.h"
#include "FSM.h"

#define HOUR 0
#define MIN 1 
#define SEC 2
#define MER 3
#define ALL 4

// Convert time variable to format (h0)(h1):(m0)(m1):(s0)(s1) (Mer0)(Mer1)
void format_And_Output_Time(char* timeStringBuffer, uint8_t* minute_hand, uint8_t* hour_hand){
	int8_t seconds, minutes, hours, meridian, index;
	uint32_t time = Time;
	seconds = time % 100;
	time = time / 1000;
	minutes = time % 100;
	*minute_hand = minutes; // Set value of minute_hand for drawing clock hand.
	time = time / 1000;
	hours = time % 100;
	*hour_hand = hours; // Set value for drawing hour hand.
	meridian = (time / 100) % 10;
	index = 0;
	char h0 = (hours / 10) + '0';
	timeStringBuffer[index] = h0;
	index++;
	char h1 = (hours % 10) + '0';
	timeStringBuffer[index] = h1;
	index++;
	timeStringBuffer[index] = ':';
	index++;
	char m0 = (minutes / 10) + '0';
	timeStringBuffer[index] = m0;
	index++;
	char m1 = (minutes % 10) + '0';
	timeStringBuffer[index] = m1;
	index++;
	timeStringBuffer[index] = ':';
	index++;
	char s0 = (seconds / 10) + '0';
	timeStringBuffer[index] = s0;
	index++;
	char s1 = (seconds % 10) + '0';
	timeStringBuffer[index] = s1;
	index++;
	timeStringBuffer[index] = ' ';
	index++;
	char Mer1 = 'M';
	char Mer0;
	if(meridian){
		Mer0 = 'P';
	} else{
		Mer0 = 'A';
	}
	timeStringBuffer[index] = Mer0;
	index++;
	timeStringBuffer[index] = Mer1;
}

void format_Time(char* timeStringBuffer){
	uint8_t* null1;
	uint8_t* null2;
	format_And_Output_Time(timeStringBuffer, null1, null2);
}

void format_setTime(char* timeStringBuffer, int type, int ALARM){
	int8_t seconds, minutes, hours, meridian, index;
	uint32_t time;
	if(ALARM == 0)
	{
		time = Time;
	}
	else{
		time = AlarmTime;
	}
	seconds = time % 100;
	time = time / 1000;
	minutes = time % 100;
	time = time / 1000;
	hours = time % 100;
	meridian = (time / 100) % 10;
	index = 0;
	
	if(type == HOUR)
	{
		char h0 = (hours / 10) + '0';
		timeStringBuffer[index] = h0;
		index++;
		char h1 = (hours % 10) + '0';
		timeStringBuffer[index] = h1;
		return;
	}
	else if(type == MIN)
	{
		char m0 = (minutes / 10) + '0';
		timeStringBuffer[index] = m0;
		index++;
		char m1 = (minutes % 10) + '0';
		timeStringBuffer[index] = m1;
		return;
	}
	else if(type == SEC)
	{
		char s0 = (seconds / 10) + '0';
		timeStringBuffer[index] = s0;
		index++;
		char s1 = (seconds % 10) + '0';
		timeStringBuffer[index] = s1;
	}
	
	else if(type == MER)
	{
		char Mer1 = 'M';
		char Mer0;
		if(meridian){
			Mer0 = 'P';
		} else{
			Mer0 = 'A';
		}
		timeStringBuffer[index] = Mer0;
		index++;
		timeStringBuffer[index] = Mer1;
		return;
	}
}
