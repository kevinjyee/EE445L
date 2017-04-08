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
	
	
	
	/****************ST7735_sDecOut2***************
 converts fixed point number to LCD
 format signed 16-bit with resolution 0.01
 range 0.00 to 45.00
 Inputs:  unsigned 16-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    "23.45"  
  3100    "31.00" 
 */ 
void ST7735_uDecOut2(uint16_t n)
	{
		int MAX = 4500; //max upper bound
		int MIN = 0; //min upper bound
	
		int MAX_DIGITS = 6; //Maximum digit of fixed point number 
		int DECIMAL_POSITION = 3; //Position of Decimal Point to be placed
		char buffer [6] = {' '}; //Initialize array to empty string 
		int numDigit;
		
		
		//check if within bounds
		if(n > MAX || n < MIN)
		{
			 ST7735_OutString("**.**");
			 return;
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
	 														 
		