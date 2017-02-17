/* File Name:    main.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      02/069/2017
 * Description:  Main functions to test for plotting floating pts, fixed pts and images
 *               
 * 
 * Lab Number: MW 330-500
 * TA: Mahesh
 * 
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
 *		 Down Switch				 PE0
 *		 Up Switch					 PE1
 *		 Select Switch			 PE2
 *		 Main Switch				 PE3
 *		 Speaker Input			 PB6
 
 */
#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "fixed.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOqueue.h"
#include "FSM.h"
#include "Clock.h"
#include "LCD.h"
#include "PWM.h"
#include "Timer0A.h"
#include "Timer3.h"


#define PA3							(*((volatile uint32_t *)0x40004020)) // Menu switch
#define PA2             (*((volatile uint32_t *)0x40004010)) // Select switch
#define PA1             (*((volatile uint32_t *)0x40004008)) // Up switch
#define PA0							(*((volatile uint32_t *)0x40004004)) // Down switch
#define SYSTICK_RELOAD	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.

#define HOUR 0
#define MIN 1 
#define SEC 2
#define MER 3
#define ALL 4
#define NUMALARMS 8

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//volatile uint32_t Switch1 = 0;
volatile uint32_t Time = 0; // (Meridian)hh0mm0ss
volatile uint32_t AlarmTime =0;
volatile uint8_t SelectSeconds;
volatile uint8_t SelectMinutes;
volatile uint8_t SelectHours;
volatile uint8_t SelectMeridian;
volatile unsigned long LastE = 0; 

uint32_t Tempo[10] = { 60, 80, 100, 120, 140, 160, 180, 200, 220, 240 };

volatile bool animateAlarm = false;
volatile bool resetClock;
extern int AlarmOn;

/*Function: getTime
* Retrieves seconds, minutes, and hour from Time global variable in thread-safe fashion.
* Consider switching to bit operations to avoid overhead of division.
*/
void getTime(uint8_t* meridian, uint8_t* hours, uint8_t* minutes, uint8_t* seconds){
	uint32_t time = Time;
	*seconds = time % 100;
	time = time / 1000;
	*minutes = time % 100;
	time = time / 1000;
	*hours = (time / 1000000) % 100;
	time = time / 100;
	*meridian = time % 10;
	
}

/*Function: DelayWait2ms
*
*/
void DelayWait2ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 7272400*4/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

/*Function: sound_On
*
*/
void sound_On(){
	Timer3_Init(Tempo[4]);
}


/*Function: sound_Off
*
*/
void sound_Off(){
	Disable_Timer3();
	Disable_PWM();
}

/*Function: init_All
*Initializes all the Timers and switches
*/
void init_All(){
	PLL_Init(Bus50MHz);                   // 50 MHz
	init_switchmain();
	ST7735_InitR(INITR_REDTAB);
  SysTick_Init(SYSTICK_RELOAD);
}

/*Function: find_minAlarm
*/
uint32_t find_minAlarm(){
	uint32_t MINALARM =0XFFFFFFFF;
  int index = -1;
	bool foundalarm = false;
	
	for(int i =0; i < NUMALARMS; i++)
	{
		if(AlarmONOFFArray[i] == 1)
		{
			if(AlarmTimeArray[i] - Time < MINALARM)
			{
			MINALARM = AlarmTimeArray[i];
			index = i;
			}
			foundalarm = true;	
		}		
	}//find closest alarm time
	
	if(foundalarm)
	{
		AlarmOn = 1;
	}
	else{
		AlarmOn = 0;
	}
return index;
}

/*Function: check_Alarm
*/
void check_Alarm(uint32_t current_state){
	if(current_state == 0x00)
		{
		draw_Time(); // Start updating time.
			uint32_t minalarm = find_minAlarm();
			if(AlarmOn == 1)
			{
				ST7735_OutString(" Alarm");
				ST7735_OutUDec(minalarm);
				ST7735_OutString("On");
			}
		}
		for(int i =0; i < NUMALARMS; i++)
		{
			if(Time == AlarmTimeArray[i] && AlarmONOFFArray[i] == 1)
		{
			sound_On();
			AlarmONOFFArray[i] = 0;
			animateAlarm = true;
		}
			
		}
}

/*Function: animate_Alarm
*/
void animate_Alarm(int* mute,uint8_t* alarm_triggered,uint32_t* current_state){
	
	Enable_Timer1(); // Enable screen timeout.
				mute = 0;
				animateAlarm = false;
				AlarmOn = 0;
			  sound_Off();
				clear_Screen();
				if(current_state == 0x00){
					redrawHands = 1;
					draw_Clock();
					draw_Time();
				}
				alarm_triggered = 0;
				resetClock = true;
}




int main(void){
  init_All();
	draw_Clock();
	EnableInterrupts();
	uint32_t current_state = 0x00;	
  uint32_t input,lastinput = 0x00;
	uint8_t alarm_triggered = 0;
	int mute = 0;
	while(1){
		check_Alarm(current_state);
			if(animateAlarm){
				Disable_Timer1(); // Disable screen timeout.
			}
			while(animateAlarm && !mute)
			{
				alarm_triggered = 1;
				mute = animate_Clock();		
			}
			if(alarm_triggered){
				animate_Alarm(&mute,&alarm_triggered,&current_state);
			}
			
		if(Fifo_Get(&input))
		{
			current_state = Next_State(current_state, input);			
			lastinput = input;
		 
		}
		else
		{
			current_state = Next_State(current_state,0x00);	
		}
  }
}




