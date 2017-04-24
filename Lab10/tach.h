/* File Name:    tach.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      April 11th, by Stefan Bordovsky
 * Description:  This program contains initialization code and functions for
 *								reading input from a tachometer.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 4/11/2017	
 */

#include "stdint.h"

extern volatile uint32_t Period;

//------------Tach_Init------------
// Initialize tachometer to read data in via ADC0 (PB7)
// Input: none
// Output: none
void Tach_Init(void);

// ***************** Tach_Read ****************
// Reads input from the tachometer.
// Inputs:  none
// Outputs: uint16_t data value read via Input Capture
uint16_t Tach_Read(void);


