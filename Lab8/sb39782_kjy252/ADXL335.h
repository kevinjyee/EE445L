// ADXL335.h
// This software configures the ADC/Accelerometer interface.
// Runs on M4C123
// Sets up three ADC ports to read ADXL335 acceleration data.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

// Lab number: 8
// Software Drivers

// Header files contain the prototypes for public functions
// this file explains what the module does

#include <stdint.h>

// **************ADC_Init*********************
// Initialize ADCs and FIFO data collection. Called once.
// Input: none 
// Output: none
void ADC_Init(void);

// **************ADC_In*********************
// Read acceleration data and compute current walking speed based on
//		new input and step rate history (uses rolling average).
// Input: none 
// Output: A speed value from 0 to 255
uint8_t ADC_In(void);

// ***************** Timer3Arm ****************
// Arm Timer3 to begin reading from accelerometer.
// Inputs:  none
// Outputs: none
void Timer3Arm(void);
