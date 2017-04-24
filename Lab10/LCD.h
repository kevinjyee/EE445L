// LCD.h
// Runs on LM4F120/TM4C123
// A software function to drive graphics for Lab 3 Alarm Clock.
// Stefan Bordovsky and Kevin Yee
// February 7, 2017

#include <stdint.h>
#include "PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"


/**************LCD_Init***************
	Initialize the ST7735 and set plot parameters.
	Inputs: none
	Outputs: none
**************************************/
void LCD_Init();

/**************Clear_Screen***************
	Colors the entire LCD screen black.
	Inputs: none
	Outputs: none
*******************************************/
void Clear_Screen();

/**************ST7735_XYplotInit************************************
 Initialize plot area on ST7735 LCD.
 Inputs:  minY   32-bit fixed point data, resolution = 0.1.
          maxY   32-bit fixed point data, resolution = 0.1.
 Outputs: none
 Allows for fixed-point data to be mapped to LCD pixels based on the
	full range of y data.
********************************************************************/
void ST7735_XYplotInit(int32_t minY, int32_t maxY);

/**************ST7735_printData***************
 Prints out RPS of motor approximated by duty cycle calculation
	and prints out RPS as measured by tachometer.
 Inputs:  pwmdata   16-bit fixed point data, resolution = 0.1.
          tachdata   16-bit fixed point data, resolution = 0.1.
 Outputs: none
*****************************************************/
void ST7735_printData(uint16_t pwmdata,uint16_t tachdata);

/**************ST7735_printDataErr***************
 Debug printsout of duty cycle-approximated motor RPS, tachomter-measured
	RPS, and error between the two readings.
 Inputs:  pwmdata   16-bit fixed point data, resolution = 0.1.
          tachdata   16-bit fixed point data, resolution = 0.1.
					Error			32-bit fixed point data, resolution = 0.1.
 Outputs: none
***************************************************/
void ST7735_printDataErr(uint16_t pwmdata,uint16_t tachdata, int32_t Error);

/**************ST7735_plotData***************
 Plots RPS of motor approximated by duty cycle calculation
	and plots RPS as measured by tachometer.
 Inputs:  pwmdata   16-bit fixed point data, resolution = 0.1.
          tachdata   16-bit fixed point data, resolution = 0.1.
 Outputs: none
***************************************************/
void ST7735_plotData(uint16_t pwmdata,uint16_t tachdata);