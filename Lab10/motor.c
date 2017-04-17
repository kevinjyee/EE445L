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
#include "UART.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


#define INITIAL_PERIOD 40000

volatile uint16_t PWMSPEED = 200;
volatile int32_t Duty=0;


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
	PWM0A_Init(INITIAL_PERIOD, 20000);          // initialize PWM0, 1000 Hz, 50% duty
	Set_Motor_Speed(20000);
	Timer3_Init(80000);
}

//------------Adjust_Motor_Speed--------
// Set the speed of the DC motor. Can be used to start the motor.
// Input: Motor speed which corresponds to a PWM duty cycle.
// Output: None.
void Adjust_Motor_Speed(void){
	//TODO: Move this to a timer handler to adjust speed
		
		//Duty = duty;
		//PWMSPEED = speed;
	  PWM0A_Duty(Duty);
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


void Debug2(pwmcurrentRPS,tachcurrentRPS,duty,measuredperiod)
{
	UART_OutString("PWM_Speed: ");
	UART_OutUDec(pwmcurrentRPS);
	UART_OutString("\n");
	
	UART_OutString("Tach_Speed: ");
	UART_OutUDec(tachcurrentRPS);
	UART_OutString("\n");
	
	UART_OutString("Duty Cycle: ");
	UART_OutUDec(duty);
	UART_OutString("\n");
	
	UART_OutString("Period: ");
	UART_OutUDec(measuredperiod);
	UART_OutString("\n");
}




uint32_t MeasuredPeriod;
uint32_t TACHSpeed;
volatile int32_t Error = 0;
#define TACH_ARR_SIZE 64
uint32_t tach_avg_arr[TACH_ARR_SIZE];
int i =0;
void average_array(void)
{
	while(i < TACH_ARR_SIZE)
	{
		
		
	}
	
}


/*
Page 330 of Book
*/
void Timer3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
	MeasuredPeriod = Period;
	TACHSpeed = (800000000/MeasuredPeriod) / 4; //0.1 RPS;

	Error =  PWMSPEED - TACHSpeed;
	int32_t absError;
	if(Error < 0){
		absError *= -1;
	} else{
		absError = Error;
	}
	if(absError < 1000){
		Duty = Duty + (12 * Error)/64; //discrete integral
	}
	//Debug2(PWMSPEED,TACHSpeed,Duty,MeasuredPeriod);
	if(Duty < 40){
		Duty = 40;
	}
	if(Duty > 39960){
		Duty = 39960;
	}
	//TODO: Modify Duty cycle from here
	Adjust_Motor_Speed();
	
}

int32_t Read_Duty(void)
{
	return Duty;
}


//------------Set_Motor_Speed--------
// Set the speed of the DC motor. Can be used to start the motor.
// Input: Motor speed which corresponds to a PWM duty cycle.
// Output: None.
void Set_Motor_Speed(uint16_t speed){
	//TODO: Move this to a timer handler to adjust speed
		
		Duty = speed*100;
		PWMSPEED = speed;
	  PWM0A_Duty(Duty);
}

