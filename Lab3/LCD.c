// LCD.c
// Runs on LM4F120/TM4C123
// A software function to drive graphics for Lab 3 Alarm Clock.
// Stefan Bordovsky and Kevin Yee
// February 7, 2017

#include <stdint.h>
#include "PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "LCD.h"
#include "Clock.h"
#include "Display.h"
#include "LCD.h"
#include "FSM.h"

#define CIRCLE_OFFSET		89

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// 180 points on a circle for minute hand.
volatile int32_t MinuteHandXbuf[180] = { -45, -87, -128, -169, -211, -252, -293, -333, -373, -413, -452, -490, -529, -566, -603, -639, -673, -708, -741, -774, -805, -836, -866, -894, -922, -948, -973, -997, -1020, -1042, -1062, -1081, -1099, -1115, -1130, -1144, -1156, -1167, -1176, -1185, -1191, -1196, -1200, -1202, -1203, -1202, -1200, -1196, -1191, -1185, -1176, -1167, -1156, -1144, -1130, -1115, -1099, -1081, -1062, -1042, -1020, -997, -973, -948, -922, -894, -866, -836, -805, -774, -741, -708, -673, -639, -603, -566, -529, -490, -452, -413, -373, -333, -293, -252, -211, -169, -128, -87, -45, -3, 39, 81, 122, 163, 205, 246, 287, 327, 367, 407, 446, 484, 523, 560, 597, 633, 667, 702, 735, 768, 799, 830, 860, 888, 916, 942, 967, 991, 1014, 1036, 1056, 1075, 1093, 1109, 1124, 1138, 1150, 1161, 1170, 1179, 1185, 1190, 1194, 1196, 1197, 1196, 1194, 1190, 1185, 1179, 1170, 1161, 1150, 1138, 1124, 1109, 1093, 1075, 1056, 1036, 1014, 991, 967, 942, 916, 888, 860, 830, 799, 768, 735, 702, 667, 633, 597, 560, 523, 484, 446, 407, 367, 327, 287, 246, 205, 163, 122, 81, 39, -3
};
volatile int32_t MinuteHandYbuf[180] = { -1343, -1341, -1337, -1332, -1326, -1317, -1308, -1297, -1285, -1271, -1256, -1240, -1222, -1203, -1183, -1161, -1138, -1114, -1089, -1063, -1035, -1007, -977, -946, -915, -882, -849, -814, -780, -744, -707, -670, -631, -593, -554, -514, -474, -434, -393, -352, -310, -269, -228, -186, -144, -102, -60, -18, 22, 64, 105, 146, 186, 226, 266, 305, 343, 382, 419, 456, 492, 526, 561, 594, 627, 658, 689, 719, 747, 775, 801, 826, 850, 873, 895, 915, 934, 952, 968, 983, 997, 1009, 1020, 1029, 1038, 1044, 1049, 1053, 1055, 1056, 1055, 1053, 1049, 1044, 1038, 1029, 1020, 1009, 997, 983, 968, 952, 934, 915, 895, 873, 850, 826, 801, 775, 747, 719, 689, 658, 627, 594, 561, 526, 492, 456, 419, 382, 343, 305, 266, 226, 186, 146, 105, 64, 22, -18, -60, -102, -144, -186, -228, -269, -310, -352, -393, -434, -474, -514, -554, -593, -631, -670, -707, -744, -780, -814, -849, -882, -915, -946, -977, -1007, -1035, -1063, -1089, -1114, -1138, -1161, -1183, -1203, -1222, -1240, -1256, -1271, -1285, -1297, -1308, -1317, -1326, -1332, -1337, -1341, -1343, -1344
};

// 180 points on a circle for hour hand. Offset = -45.
volatile int32_t HourHandXbuf[180] = { -30, -58, -85, -113, -140, -168, -195, -222, -249, -275, -301, -327, -352, -377, -402, -426, -449, -472, -494, -516, -537, -557, -577, -596, -614, -632, -649, -665, -680, -694, -708, -721, -732, -743, -753, -762, -771, -778, -784, -790, -794, -797, -800, -801, -802, -801, -800, -797, -794, -790, -784, -778, -771, -762, -753, -743, -732, -721, -708, -694, -680, -665, -649, -632, -614, -596, -577, -557, -537, -516, -494, -472, -449, -426, -402, -377, -352, -327, -301, -275, -249, -222, -195, -168, -140, -113, -85, -58, -30, -2, 26, 54, 81, 109, 136, 164, 191, 218, 245, 271, 297, 323, 348, 373, 398, 422, 445, 468, 490, 512, 533, 553, 573, 592, 610, 628, 645, 661, 676, 690, 704, 717, 728, 739, 749, 758, 767, 774, 780, 786, 790, 793, 796, 797, 798, 797, 796, 793, 790, 786, 780, 774, 767, 758, 749, 739, 728, 717, 704, 690, 676, 661, 645, 628, 610, 592, 573, 553, 533, 512, 490, 468, 445, 422, 398, 373, 348, 323, 297, 271, 245, 218, 191, 164, 136, 109, 81, 54, 26, -2
};
volatile int32_t HourHandYbuf[180] = { -1019, -1018, -1015, -1012, -1008, -1002, -996, -989, -980, -971, -961, -950, -939, -926, -912, -898, -883, -867, -850, -832, -814, -795, -775, -755, -734, -712, -690, -667, -644, -620, -595, -570, -545, -519, -493, -467, -440, -413, -386, -358, -331, -303, -276, -248, -220, -192, -164, -136, -108, -81, -53, -26, 0, 27, 53, 79, 105, 130, 155, 180, 204, 227, 250, 272, 294, 315, 335, 355, 374, 392, 410, 427, 443, 458, 472, 486, 499, 510, 521, 531, 540, 549, 556, 562, 568, 572, 575, 578, 579, 580, 579, 578, 575, 572, 568, 562, 556, 549, 540, 531, 521, 510, 499, 486, 472, 458, 443, 427, 410, 392, 374, 355, 335, 315, 294, 272, 250, 227, 204, 180, 155, 130, 105, 79, 53, 27, 0, -26, -53, -81, -108, -136, -164, -192, -220, -248, -276, -303, -331, -358, -386, -413, -440, -467, -493, -519, -545, -570, -595, -620, -644, -667, -690, -712, -734, -755, -775, -795, -814, -832, -850, -867, -883, -898, -912, -926, -939, -950, -961, -971, -980, -989, -996, -1002, -1008, -1012, -1015, -1018, -1019, -1020
};

volatile uint8_t lastMinute = 61;
volatile uint8_t lastHour = 13;

volatile uint16_t lastDrawnMinuteIndex = 0;
volatile uint16_t lastDrawnHourIndex = 0;

volatile uint8_t newly_Loaded = 1;
volatile int redrawHands = 1;

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
static int32_t MinX, MaxX, MinY, MaxY;
void ST7735_XYplotInit1(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
	//sets static variable
	MinX = minX; MaxX = maxX;
  MinY = minY; MaxY = maxY;
	
	//clear screen and output title
	
	//ST7735_FillScreen(0);
	//ST7735_SetCursor(0,0);
  //ST7735_OutString(title);
}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot1(uint32_t num, int32_t bufX[], int32_t bufY[]){

	/*
	In this code, the plotting area is a square on the bottom (0,32) to (127,159)
   i goes from 0 to 127
   x=MaxX maps to i=0
   x=MaxX maps to i=127
  i = (127*(x-MinX))/(MaxX-MinX);  
   y=MaxY maps to j=32
   y=MinY maps to j=159
  j = 32+(127*(MaxY-y))/(MaxY-MinY);
*/
	

	for (uint32_t i=0 ; i < num; ++i) {
		if (bufX[i] < MinX   ||   bufX[i] > MaxX   ||   
				bufY[i] < MinY   ||   bufY[i] > MaxY) { continue; } 
		else
			{
				int32_t x = (127*(bufX[i] - MinX)) / (MaxX - MinX);
				int32_t y = 32+(127*((MaxY - bufY[i]))  / (MaxY - MinY));

				ST7735_DrawPixel(x,   y,   ST7735_CYAN);
				ST7735_DrawPixel(x+1, y,   ST7735_CYAN);
				ST7735_DrawPixel(x,   y+1, ST7735_CYAN);
				ST7735_DrawPixel(x+1, y+1, ST7735_CYAN);
			}
    
	}
}

/**************ST7735_Translate***************
 Translate an array of (x,y) data from fixed-point to LCD-mapped values.
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_Translate1(uint32_t num, int32_t bufX[], int32_t bufY[]){	

	for (uint32_t i=0 ; i < num; ++i) {
		if (bufX[i] < MinX   ||   bufX[i] > MaxX   ||   
      bufY[i] < MinY   ||   bufY[i] > MaxY) { 
			bufX[i] = -1; // Indicate out-of-bounds point with -1.
			bufY[i] = -1;
		} 
		else
		{
      bufX[i] = (127*(bufX[i] - MinX)) / (MaxX - MinX);
      bufY[i] = 32+(127*((MaxY - bufY[i]))  / (MaxY - MinY));
		}
    
  }
}

//************* ST7735_Line********************************************
//  Draws one line on the ST7735 color LCD using Bresenham's Line Algorithm.
//  Inputs: (x1,y1) is the start point
//          (x2,y2) is the end point
// x1,x2 are horizontal positions, columns from the left edge
//               must be less than 128
//               0 is on the left, 126 is near the right
// y1,y2 are vertical positions, rows from the top edge
//               must be less than 160
//               159 is near the wires, 0 is the side opposite the wires
//        color 16-bit color, which can be produced by ST7735_Color565() 
// Output: none
// Note: This is an implementation of Bresenham's algorithm inspired by an example
//		from rosettacode.org
void ST7735_Line1(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
	{
		int xDif, yDif, signX, signY, error, delta_error;
		xDif = x2 - x1;
		xDif = xDif > 0 ? xDif : -xDif; // Take absolute value of xDif.
		yDif = y2 - y1;
		yDif = yDif > 0 ? yDif : -yDif; // Take absolute value of yDif.
		signX = x2 > x1 ? 1 : -1;
		signY = y2 > y1 ? 1 : -1;
		if(xDif > yDif){
			error = xDif / 2;
		} else{
			error = -yDif / 2;
		}
		while((x1 != x2) || (y1 != y2)){
			ST7735_DrawPixel(x1,   y1,   color);
      ST7735_DrawPixel(x1+1, y1,   color);
      ST7735_DrawPixel(x1,   y1+1, color);
      ST7735_DrawPixel(x1+1, y1+1, color);
			delta_error = error;
			if(delta_error > -xDif){
				error -= yDif;
				x1 += signX;
			}
			if(delta_error < yDif){
				error += xDif;
				y1 += signY;
			}
		}
	}

uint16_t get_Minute_Hand_Index(uint8_t minutes){
	return (((minutes * 3) + CIRCLE_OFFSET) % 180);
}

uint16_t get_Hour_Hand_Index(uint8_t minutes, uint8_t hours){
	return(((((hours % 12) * 15) + ((minutes / 12) * 3)) + CIRCLE_OFFSET) % 180);
}

uint8_t indicesAreClose(uint16_t prevMinutesIndex, uint16_t hoursIndex){
	int16_t diff = prevMinutesIndex - hoursIndex;
	if(diff < 0){
		diff *= -1;
	}
	return (diff < 12);
}

void draw_Hands(uint8_t minutes, uint8_t hours){
	// Origin ~ 63x, 99y
		uint16_t minutesIndex = get_Minute_Hand_Index(minutes);
		//uint16_t prevMinutesIndex = (minutesIndex >= 3) ? minutesIndex - 3 : (177 + minutesIndex); // Don't want negative index.
		uint16_t hoursIndex = get_Hour_Hand_Index(minutes, hours);
		//uint16_t prevHoursIndex = (hoursIndex >= 3) ? hoursIndex - 3 : (177 + hoursIndex); // Don't want negative index.
		ST7735_Line1(63, 99, MinuteHandXbuf[lastDrawnMinuteIndex], MinuteHandYbuf[lastDrawnMinuteIndex], ST7735_BLACK); // Delete old minute hand
		if((hoursIndex != lastDrawnHourIndex)){
			ST7735_Line1(63, 99, HourHandXbuf[lastDrawnHourIndex], HourHandYbuf[lastDrawnHourIndex], ST7735_BLACK); // Delete old hour hand
		}
			ST7735_Line1(63, 99, MinuteHandXbuf[minutesIndex], MinuteHandYbuf[minutesIndex], ST7735_GREEN); // Draw new minute hand
			lastDrawnMinuteIndex = minutesIndex;
		if((hoursIndex != lastDrawnHourIndex) || indicesAreClose(lastDrawnMinuteIndex, hoursIndex) || redrawHands){
			/* Draw new hour hand if minutes = multiple of 12, if the hours have been changed, or if the minute hand is close enough to
			 		the hour hand that its line clear method might delete part of the hour hand.
			*/
			ST7735_Line1(63, 99, HourHandXbuf[hoursIndex], HourHandYbuf[hoursIndex], ST7735_GREEN); // Draw new hour hand
			lastDrawnHourIndex = hoursIndex;
		}
	
		
	/*
		for(int i = 0; i < 180; i++){
			ST7735_DrawPixel(MinuteHandXbuf[i],   MinuteHandYbuf[i],   ST7735_CYAN);
		}
		//ST7735_Translate1(180, (int32_t*) HourHandXbuf, (int32_t*) HourHandYbuf); // Translate circle to screen for line drawing.
		for(int i = 0; i < 180; i++){
			ST7735_DrawPixel(HourHandXbuf[i],   HourHandYbuf[i],   ST7735_CYAN);
		}
		*/
	
}	

void clear_OldHands(){
	DisableInterrupts();
	uint8_t minutes = lastMinute;
  uint8_t hours = lastHour;
	uint16_t minutesIndex = get_Minute_Hand_Index(minutes);
	uint16_t prevMinutesIndex = (minutesIndex >= 3) ? minutesIndex - 3 : (177 + minutesIndex); // Don't want negative index.
		uint16_t hoursIndex = get_Hour_Hand_Index(minutes, hours);
		uint16_t prevHoursIndex = (hoursIndex >= 3) ? hoursIndex - 3 : (177 + hoursIndex); // Don't want negative index.

	
		ST7735_Line1(63, 99, MinuteHandXbuf[prevMinutesIndex], MinuteHandYbuf[prevMinutesIndex], ST7735_BLACK); // Delete old minute hand

			ST7735_Line1(63, 99, HourHandXbuf[prevHoursIndex], HourHandYbuf[prevHoursIndex], ST7735_BLACK); // Delete old hour hand
		
			ST7735_Line1(63, 99, MinuteHandXbuf[minutesIndex], MinuteHandYbuf[minutesIndex], ST7735_BLACK); // Draw new minute hand

			/* Draw new hour hand if minutes = multiple of 12, if the hours have been changed, or if the minute hand is close enough to
			 		the hour hand that its line clear method might delete part of the hour hand.
			*/
			ST7735_Line1(63, 99, HourHandXbuf[hoursIndex], HourHandYbuf[hoursIndex], ST7735_BLACK); // Draw new hour hand
	
		
		EnableInterrupts();
	}

void draw_Time(){
	uint8_t minutes;
  uint8_t hours;
	char timeStringBuffer[10] = {' '}; //Initialize array to empty string 
	format_And_Output_Time(timeStringBuffer, &minutes, &hours);

	
	if(redrawHands || (minutes != lastMinute) || (hours != lastHour)){
		draw_Hands(minutes, hours);
		redrawHands = 0;
		lastMinute = minutes;
		lastHour = hours;
	}

	ST7735_SetCursor(0,0);
  ST7735_OutString(timeStringBuffer);
	
	//ST7735_OutUDec(difference);
	
}

void DelayWait1Millisecond(uint32_t n){
	uint32_t volatile time;
  while(n){
    time = 7272400*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

void animate_Clock(){
	for(int i = 160; i > 0; i--){
			
		ST7735_DrawBitmap(0,159,ClockFace,i,160);
		DelayWait1Millisecond(1);
	}
}

void initialize_Hands(void){
	ST7735_XYplotInit1(-2500, 2500, -2500, 2500); //convert maxxX and maxxY to thousands based on resoltuion
	ST7735_Translate1(180, (int32_t*) MinuteHandXbuf, (int32_t*) MinuteHandYbuf); // Translate circle to screen for line drawing.
	ST7735_Translate1(180, (int32_t*) HourHandXbuf, (int32_t*) HourHandYbuf); // Translate circle to screen for line drawing.
}

void draw_Clock(void){
	ST7735_DrawBitmap(0,159,ClockFace,128,160);
	if(newly_Loaded){
		initialize_Hands();
		newly_Loaded = 0;
	}
	//animate_Clock();
	//ST7735_DrawBitmap(2,159,ClockFace,126,160); Inspiration for sideways clock?

	//draw_Hands();
}