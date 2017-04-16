/* File Name:    motor.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      April 11th, by Stefan Bordovsky
 * Description:  This program contains initialization code and functions for
 *								controlling a DC motor with PWM.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 4/11/2017	
 */

#include "stdint.h"
#include "PWM.h"
#include "motor.h"
#include "tach.h"
#include "../inc/tm4c123gh6pm.h"



void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


#define INITIAL_PERIOD 40000

volatile uint16_t PWMSPEED = INITIAL_PERIOD/2;


void Timer3_Init(uint32_t period){
	DisableInterrupts();
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = period-1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear TIMER3A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 51, interrupt number 35
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
	EnableInterrupts();
}

//------------Motor_Init------------
// Initialize motor by initializing PWM.
// Input: none
// Output: none
void Motor_Init(){
	PWM0A_Init(INITIAL_PERIOD, INITIAL_PERIOD/2);          // initialize PWM0, 1000 Hz, 50% duty
	//Timer3_Init(400000);
}

//------------Set_Motor_Speed--------
// Set the speed of the DC motor. Can be used to start the motor.
// Input: Motor speed which corresponds to a PWM duty cycle.
// Output: None.
void Set_Motor_Speed(uint16_t speed){
	//TODO: Move this to a timer handler to adjust speed

		PWMSPEED = speed;
}

//------------Stop_Motor-------------
// Stop the DC motor.
// Input: none
// Output: none
void Stop_Motor(){
	PWM0A_Duty(0);
}

// ***************** Motor_Test ****************
// Tests the motor
// Inputs:  none
// Outputs: none
void Motor_Test(void);


uint32_t MeasuredPeriod;
uint32_t TACHSpeed;
int32_t Error;
int32_t Duty;

void Timer3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
	MeasuredPeriod = Tach_Read();
	TACHSpeed = 800000000/MeasuredPeriod; //0.1 RPS;
	Error = TACHSpeed - PWMSPEED;
  Duty = Duty + (3 * Error)/64; //discrete integral
	if(Duty < 40){
		Duty = 40;
	}
	if(Duty > 39960){
		Duty = 39960;
	}
	PWM0A_Duty(Duty);
}