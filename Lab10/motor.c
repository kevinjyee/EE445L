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

#define INITIAL_PERIOD 40000

volatile uint16_t CURRENT_DUTY = INITIAL_PERIOD/2;

//------------Motor_Init------------
// Initialize motor by initializing PWM.
// Input: none
// Output: none
void Motor_Init(){
	PWM0A_Init(INITIAL_PERIOD, INITIAL_PERIOD/2);          // initialize PWM0, 1000 Hz, 50% duty
}

//------------Set_Motor_Speed--------
// Set the speed of the DC motor. Can be used to start the motor.
// Input: Motor speed which corresponds to a PWM duty cycle.
// Output: None.
void Set_Motor_Speed(uint16_t speed){
	//TODO: Move this to a timer handler to adjust speed
	//PWM0A_Duty(speed);
	CURRENT_DUTY = speed;
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


