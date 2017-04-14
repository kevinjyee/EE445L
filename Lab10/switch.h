/* File Name:    switch.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      April 11th, by Stefan Bordovsky
 * Description:  This program contains initialization code and functions for
 *								a switch-to-DC motor interface. Sets up debounced switches.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 4/11/2017	
 */

#include "stdint.h"

// **************Switch_Init*********************
// Initialize Switch key inputs and FIFO data collection. Called once.
// Input: none 
// Output: none
void Switch_Init(void);

// **************Switch_In*********************
// Check switch input values.
// Input: none 
// Output: 0 to 3 depending on switches depressed.
// 0x01 is just Switch0, 0x02 is just Switch1, 0x03 is Switch0, Switch1
uint8_t Switch_In(void);
