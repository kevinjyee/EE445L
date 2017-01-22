// fixed.c
// Lab 1 Spring 2017
// Kevin Yee
// 01/19/2017

#include <stdlib.h>
#include <stdint.h>
#include "fixed.h"
#include "ST7735.h"

#define TRUE 1
#define FALSE 0



/****************numDigits***************
 changes a string to an appropriate LCD ST7735 format
 dependant on maximum digits and decimal positions
 */ 
 
 int numDigits(int32_t n)
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
		int max = 9999;
		int min = -9999;
		float RESOLUTION = 0.001;
		int MAX_DIGITS = 6;
		int DECIMAL_POSITION = 2;
		char buffer [6] = {' '};
		int numDigit;
		
		
		//check if within bounds
		if(n > max || n < min)
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
		
		numDigit = numDigits(n);
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
	
	int max = 255999;
	int min = 0;
	double RESOLUTION = 256;
	double num = (double) n;
	int MAX_DIGITS = 6;
	int DECIMAL_POSITION = 3;
	char buffer [6] = {' '};
	double roundedResult;
	int scaledResult =0;
	int numDigit;
	
	
	if( n > max || n < min)
	{
		ST7735_OutString("***.**");
		return;
	}
	
	roundedResult = ((double)((double) num / RESOLUTION*100)/100);
	
	scaledResult = roundedResult * 100;
	numDigit = numDigits(n);
	change_To_Output( scaledResult, MAX_DIGITS, DECIMAL_POSITION,  buffer,numDigit,FALSE);
	
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
	MinX = minX; MaxX = maxX;
  MinY = minY; MaxY = maxY;
	ST7735_FillScreen(0);
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
	
}
