/* File Name:    PedometerScreen.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      01/19/2017
 * Description:  Prints out the pedometer screen values.
 * 
 * Lab Number: MW 330-500
 * TA: Mahesh
 * Last Revised: 4/07/2017
*/ 

#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOQueue.h"
#include "Lab8.h"
#include "LCD.h"
#include "Globals.h"
#include "FSM.h"
#include "Accelerometer.h"

void Draw_Steps()
{
	Accel_Test();
	
}
uint32_t PedometerScreen(uint32_t input){
	Draw_Title(XTITLE,YTITLE,TITLEBORDER,"Pedometer");
	//Determine Input and Action on MainScreen
	switch(input)
	{
		case LEFT:
			ST7735_FillScreen(ST7735_WHITE);
			return 0x00;
	}
	Draw_Steps();
	return rtPedometer;
}