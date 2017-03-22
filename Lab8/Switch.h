// Switch.h
// This software configures the off-board Switch keys
// Runs on LM4F120 or TM4C123
// Sets up debounced switches to serve as controls for the alarm clock.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

// Lab number: 6
// Hardware connections

// Header files contain the prototypes for public functions
// this file explains what the module does

#include <stdint.h>

// **************Switch_Init*********************
// Initialize Switch key inputs and FIFO data collection. Called once.
// Input: none 
// Output: none
void Switch_Init(void);

// **************Switch_In*********************
// Check switch input values.
// Input: none 
// Output: 0 to 7 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
uint32_t Switch_In(void);

// ***************** Timer2Arm ****************
// Arm Timer2 to begin checking for switch input.
// Inputs:  none
// Outputs: none
void Timer2Arm(void);

