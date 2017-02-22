// SysTick.h
// Runs on LM4F120/TM4C123
// Use SysTick at 50MHz to keep track of time. Each time
// 	SysTick interrupts, increment time by one decisecond. (1/10th a second)
// 	Keep a heartbeat using PF2 while SysTick is keeping the time.
// Stefan Bordovsky and Kevin Yee
// February 16, 2017

/* This example accompanies the books
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   Volume 1, Program 4.7

   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 2.11, Section 2.6

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
 
 #include <stdint.h>

extern volatile uint32_t Time;

extern volatile uint32_t AlarmTime;

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(uint32_t period);

// Set current time on clock.
void SysTick_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t meridian);

// Increment current time by one dSecond.
void incrementTime(void);

// Increment time every time handler is called.
void SysTick_Handler(void);

