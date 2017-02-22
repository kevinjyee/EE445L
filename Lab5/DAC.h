/* File Name:    DAC.h
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program serves as an interface with the DAC in order to
 * 		play music on command.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 2/08/2017	
 */
 
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../SysTick.h"

// ***************** DAC_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void DAC_Init();

// ***************** DAC_Out ****************
// Outputs a voltage value to the DAC
// Inputs:  none
// Outputs: none
void DAC_Out();