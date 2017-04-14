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

/**************Print_Data******************
	Prints out the intended motor speed and the measured tachometer speed.
	Inputs: intended_speed	The speed which at which the motor thinks it's spinning.
													32-bit fixed-point data, resolution= 0.001
					measured_speed	The speed which the tachometer thinks the motor is spinning.
													32-bit fixed-point data, resolution= 0.01
	Outputs: none
*******************************************/
void Print_Data(uint32_t intended_speed, uint32_t measured_speed);

/**************Plot_Data******************
	Plots the latest motor speed and tachometer measurements.
	Inputs: intended_speed	The speed which at which the motor thinks it's spinning.
													32-bit fixed-point data, resolution= 0.01
					measured_speed	The speed which the tachometer thinks the motor is spinning.
													32-bit fixed-point data, resolution= 0.01
	Outputs: none
*******************************************/
void Plot_Data(uint32_t intended_speed, uint32_t measured_speed);
