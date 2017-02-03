/* File Name:    fixed.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      01/19/2017
 * Description:  Changes integers to strings that can be outputs in an LCD
 *               Contains function to plot points to draw shapes
 * 
 * Lab Number: MW 330-500
 * TA: Mahesh
 * Last Revised: 1/30/2017	
 * Hardware Configurations:
 * ST7735R LCD:
 *     Backlight    (pin 10) connected to +3.3 V
 *     MISO         (pin 9) unconnected
 *     SCK          (pin 8) connected to PA2 (SSI0Clk)
 *     MOSI         (pin 7) connected to PA5 (SSI0Tx)
 *     TFT_CS       (pin 6) connected to PA3 (SSI0Fss)
 *     CARD_CS      (pin 5) unconnected
 *     Data/Command (pin 4) connected to PA6 (GPIO)
 *     RESET        (pin 3) connected to PA7 (GPIO)
 *     VCC          (pin 2) connected to +3.3 V
 *     Gnd          (pin 1) connected to ground
 */

#include <stdlib.h>
#include <stdint.h>
#include "fixed.h"
#include "ST7735.h"

#define TRUE 1
#define FALSE 0



/****************numDigits***************
 counts the number of digits in an integer 
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: digits in the number
 */ 
 
 int num_Digits(int32_t n)
 {
	 int numDigit =0;
	 while(n!=0)
	 {
		 n=n/10;
		 numDigit++;
	 }
	 return numDigit;
 }
 
/****************change_To_Output***************
 changes a string to an appropriate LCD ST7735 format
 dependant on maximum digits and decimal positions
 Inputs:  signed 32-bit integer, number of digits to print, decimal position,
					char array to hold string, number of digits of the integer, boolean value determing 
					unsigned or signed integers
 Outputs: String in a char array
 */ 
 
 char* change_To_Output(int32_t n, int MAX_DIGITS, int DECIMAL_POSITION, char buffer[], int numDigits, int signedInt)
 {
	 int i =0;
	 //create buffer translating integer to string
		for(i=MAX_DIGITS-1; i > 0; i--)
		{
			if(i != DECIMAL_POSITION)
			{
			buffer[i] = n%10 + '0'; //inserts values
			n = n/10;
				numDigits --;
			}
			else
			{
				buffer[i] = '.'; //places decimal point
			}
			if(i > numDigits && i < DECIMAL_POSITION -1  && buffer[i] == '0')
			{
				buffer[i] = ' '; //removes leading zeroes
			}				
		}
	 if(!signedInt && n != 0)
	 {
		 buffer[0] = n%10 + '0';
	 }
		 
	 return buffer;
 }

/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    " 2.345"  
 -8100    "-8.100"
  -102    "-0.102" 
    31    " 0.031" 
-12345    " *.***"
 */ 
void ST7735_sDecOut3(int32_t n)
	{
		int MAX = 9999; //max upper bound
		int MIN = -9999; //min upper bound
	
		int MAX_DIGITS = 6; //Maximum digit of fixed point number 
		int DECIMAL_POSITION = 2; //Position of Decimal Point to be placed
		char buffer [6] = {' '}; //Initialize array to empty string 
		int numDigit;
		
		
		//check if within bounds
		if(n > MAX || n < MIN)
		{
			 ST7735_OutString("*.***");
			 return;
		}
		//check if negative
		if(n < 0)
		{
				buffer[0] = '-';
				n *= -1;
		}
		else
		{
			buffer[0] = ' ';
		}
		
		numDigit = num_Digits(n); //count number of digits
		change_To_Output(n, MAX_DIGITS, DECIMAL_POSITION, buffer, numDigit,TRUE);
		
		ST7735_OutString(buffer);
		
	
	}
	
	/**************ST7735_uBinOut8***************
 unsigned 32-bit binary fixed-point with a resolution of 1/256. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 256000, it signifies an error. 
 The ST7735_uBinOut8 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
     0	  "  0.00"
     2	  "  0.01"
    64	  "  0.25"
   100	  "  0.39"
   500	  "  1.95"
   512	  "  2.00"
  5000	  " 19.53"
 30000	  "117.19"
255997	  "999.99"
256000	  "***.**"
*/
void ST7735_uBinOut8(uint32_t n){
	
	int MAX = 255999;
	int MIN = 0;
	
	int RESOLUTION = 256;

	int MAX_DIGITS = 6;
	int DECIMAL_POSITION = 3;
	char buffer [6] = {' '};
	int roundedResult;

	int numDigit;
	
	//check if value is within bounds 
	if( n > MAX || n < MIN)
	{
		ST7735_OutString("***.**");
		return;
	}
	

	roundedResult = (n*1000)/RESOLUTION; //scale up to move decimal point
	if((roundedResult % 10) >= 5){
		roundedResult+=10; //round up if the last digit is greater than 5
	}
	roundedResult /= 10; //Divide by 10 to normalize back to correct precision
	numDigit = num_Digits(n); //count number of digits 
	change_To_Output( roundedResult, MAX_DIGITS, DECIMAL_POSITION,  buffer,numDigit,FALSE);
	
	ST7735_OutString(buffer);

		
	
}

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
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
	//sets static variable
	MinX = minX; MaxX = maxX;
  MinY = minY; MaxY = maxY;
	
	//clear screen and output title
	
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
  ST7735_OutString(title);
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
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){

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


void ST7735_PlotBarXY(int32_t x, int32_t y){
	int32_t i, j;
  if (x < MinX || MaxX < x || y < MinY || MaxY < y) {
			
			return;
		}
  // X goes from 0 to 127
  // j goes from 159 to 32
  // y=Ymax maps to j=32
  // y=Ymin maps to j=159
	i = (127 * (x - MinX)) / (MaxX - MinX); 
  j = 32 + (127 * (MaxY - y)) / (MaxY - MinY);
  ST7735_DrawFastVLine(i, j, 159-j, ST7735_CYAN);

}


int find_GCD(uint16_t num1, uint16_t num2)
{
	if(num2 != 0)
	{
		return find_GCD(num2,num1%num2);
	}
	else
	{
		return num1;
	}
}

//************* ST7735_Line********************************************
//  Draws one line on the ST7735 color LCD
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
void ST7735_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
	{
		int xSlope, ySlope;
		xSlope = x2-x1;
		ySlope = y2-y1;
		int gcd = find_GCD(xSlope,ySlope);
		xSlope /= gcd;
		ySlope /= gcd;
		if(xSlope ==0)
		{
			if(ySlope < 0){ySlope *= -1;}
			ST7735_DrawFastVLine(x1,y1,ySlope,color);
		}
		else if(ySlope == 0)
		{
			if(xSlope < 0){xSlope *= -1;}
			ST7735_DrawFastHLine(x1,y1,xSlope,color);
		}
		else{
		while(x1 != x2 && y1 != y2)
		{
			ST7735_DrawPixel(x1,   y1,   color);
      ST7735_DrawPixel(x1+1, y1,   color);
      ST7735_DrawPixel(x1,   y1+1, color);
      ST7735_DrawPixel(x1+1, y1+1, color);
			x1 += gcd;
			y1 += gcd;
		}
	}							 														 
		}

		