// fixed.c
// Lab 1 Spring 2017
// Kevin Yee
// 01/19/2017

#include <stdlib.h>
#include <stdint.h>
#include "fixed.h"

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
		
		if(n > max || n < min)
		{
			//print out "*.***"
		}
	
	}