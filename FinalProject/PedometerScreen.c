#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOQueue.h"
#include "LCD.h"
#include "Globals.h"
#include "FSM.h"
#include "Accelerometer.h"
#include "Fixed.h"

extern volatile uint32_t stepCount;
extern volatile uint32_t XThresh, YThresh, ZThresh;
extern volatile uint32_t XAvg, YAvg, ZAvg;

void Draw_Steps()
{
	ST7735_DrawStringBG(0,4,"Step Count:     ",ST7735_BLACK,ST7735_WHITE); 
	
	ST7735_sDecOutNorm(Step_Count,12,4); 
/*
	ST7735_OutChar('\n');
	
	ST7735_DrawStringBG(0,5,"Averages:     ",ST7735_BLACK,ST7735_WHITE); 
		
	ST7735_SetCursor(2,6);  
		ST7735_sDecOutNorm(2,6,XAvg);
		ST7735_OutChar(' ');
		ST7735_OutUDec(YAvg);
		ST7735_OutChar(' ');
		ST7735_OutUDec(ZAvg);
		ST7735_OutChar(' ');
		ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,7,"Thresholds:     ",ST7735_BLACK,ST7735_WHITE); 
		ST7735_SetCursor(2,8);  
		ST7735_OutUDec(XThresh);
		ST7735_OutChar(' ');
		ST7735_OutUDec(YThresh);
		ST7735_OutChar(' ');
		ST7735_OutUDec(ZThresh);
		ST7735_OutChar(' ');
		ST7735_OutChar('\n');
		ST7735_DrawStringBG(0,9,"NextStepTooSoon:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(2,10);  
		ST7735_OutUDec(NextStepTooSoon);
		ST7735_DrawStringBG(0,11,"DetectionAxis:     ",ST7735_WHITE,ST7735_BLACK); 
		ST7735_SetCursor(2,12);  
		if(DetectionAxis == 1){
			ST7735_OutChar('X');
		} else if(DetectionAxis == 2){
			ST7735_OutChar('Y');
		} else if(DetectionAxis == 3){
			ST7735_OutChar('Z');
	
}
*/
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
