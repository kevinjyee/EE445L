/* File Name:    Accelerometer.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program contains initialization and data reading of the ADXL335.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 2/21/2017	
 */

#define XACCEL 0
#define YACCEL 1
#define ZACCEL 2

#include "stdint.h"

void ADC_Init321(void);

void Accel_Init(void);
//------------ADC_In321------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: two 12-bit result of ADC conversions
// Samples ADC8 and ADC9 
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// data[0] is ADC1 (PE0) 0 to 4095
// data[1] is ADC2 (PE1) 0 to 4095
// data[2] is ADC3 (PE3)
void ADC_In321(uint32_t data[3]);

// ***************** Accel_Test ****************
// Tests the accelerometer
// Inputs:  none
// Outputs: none
void Accel_Test(void);