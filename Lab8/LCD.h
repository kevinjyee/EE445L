// LCD.h
// Runs on LM4F120/TM4C123
// A software function to drive graphics for the Lab 5 Music Player.
// Stefan Bordovsky and Kevin Yee
// February 20, 2017

#include <stdint.h>
#include "PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"

void ST7735_XYplotInit(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY);

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]);
	
/**************ST7735_Translate***************
 Translate an array of (x,y) data from fixed-point to LCD-mapped values.
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_Translate(uint32_t num, int32_t bufX[], int32_t bufY[]);

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
void ST7735_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

// ***************** clear_Screen ****************
// Blank out screen.
// Inputs:  none
// Outputs: none
void clear_Screen(void);
