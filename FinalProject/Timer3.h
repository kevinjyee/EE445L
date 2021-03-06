/* File Name:    Timer3.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program initializes Timer3 to interrupt every ~100ms,
 * 		maintaining a tempo at which it calls PWM_Sine to change notes played.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 2/08/2017	
 */

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
  Program 7.5, example 7.6

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#ifndef __TIMER3INTS_H__ // do not include more than once
#define __TIMER3INTS_H__

#include "stdint.h"

// ***************** Disable_Timer3 ****************
// Disables Timer3 to stop PMWSine calls.
// Inputs:  none
// Outputs: none
void Disable_Timer3(void);

// ***************** Timer3_Init ****************
// Activate Timer3 interrupts periodically
// Inputs:  period in units (1/clockfreq)
// Outputs: none
void Timer3_Init(void(*task)(void), uint32_t period);

// ***************** Timer3A_Halt ****************
// Stops Timer3A counting down.
// Inputs:  none
// Outputs: none
void Timer3A_Halt(void);

#endif // __TIMER2INTS_H__
