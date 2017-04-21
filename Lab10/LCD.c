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
#include "fixed.h"
#include "UART.h"
#include "motor.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts


static int32_t MinX, MaxX, MinY, MaxY;

// TODO: Plot init.
void ST7735_XYplotInit(int32_t minY, int32_t maxY){

  MinY = minY; MaxY = maxY;
	
	//clear screen and output title
	
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
	ST7735_OutString("Lab 10: DC Motor");

	ST7735_PlotClear(MinY, MaxY); 
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
	
void clear_Screen()
{
	ST7735_FillScreen(0);
}



void ST7735_printData(uint16_t pwmdata,uint16_t tachdata)
{

	ST7735_SetCursor(0, 1);
	ST7735_OutString("DesiredB:");
	ST7735_uDecOut1(pwmdata);
	ST7735_OutString("RPS");
		ST7735_SetCursor(0, 2);
	ST7735_OutString("ActualR:");
	ST7735_uDecOut1(tachdata);
	ST7735_OutString("RPS");
	
}

void ST7735_printDataErr(uint16_t pwmdata,uint16_t tachdata, int32_t Error)
{

	ST7735_SetCursor(0, 1);
	ST7735_OutString("DesiredB:");
	ST7735_uDecOut1(pwmdata);
	ST7735_OutString("RPS");
		ST7735_SetCursor(0, 2);
	ST7735_OutString("ActualR:");
	ST7735_uDecOut1(tachdata);
	ST7735_OutString("RPS");
	ST7735_SetCursor(0, 3);
	ST7735_OutString("Error:");
	int32_t testError;
	if(Error < 0){
		testError = Error * -1;
		ST7735_OutString("-");
	}
	ST7735_uDecOut1((uint16_t) testError);
	ST7735_SetCursor(0, 4);
	ST7735_OutString("Duty:");
	int32_t testDuty;
	if(Duty < 0){
		testDuty = Duty * -1;
		ST7735_OutString("-");
	}
	ST7735_uDecOut1((uint16_t) testDuty/10);
}


void ST7735_plotData(uint16_t pwmdata,uint16_t tachdata)
{

	ST7735_SetTextColor(ST7735_BLUE);
	ST7735_PlotPoint(pwmdata);
	ST7735_SetTextColor(ST7735_RED);
	ST7735_PlotPointRed(tachdata);
	ST7735_PlotNextErase();
}



