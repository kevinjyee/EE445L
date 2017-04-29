// AccelAvgFIFO.h
// Runs on LM4F120/TM4C123
// Allow access to a queue for maintaining rolling acceleration average.
//	Supports Put and get_Avg queries.
// Stefan Bordovsky and Kevin Yee
// April 26th, 2016

#ifndef __queue_h__
#define __queue_h__
#include "inc/tm4c123gh6pm.h"
#include "stdint.h"


long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

// ***************** Acc_Fifo_Init ****************
// Initializes the Fifo Queue for maintaining acceleration average.
// Inputs:  none
// Outputs: none
void Acc_Fifo_Init(void);

// ***************** Acc_Fifo_Put ****************
// Puts values into the FIFO queue and modifies average.
// Inputs:  32-bit data.
// Outputs: Outputs 1 if step measured, 0 otherwise
uint8_t Acc_Fifo_Put(uint32_t X, uint32_t Y, uint32_t Z);

// *****************get_Acc_Fifo_Avg ****************
// Gets the rolling acceleration average
// Inputs:  None
// Outputs: -1 if average not fully computed, otherwise = avg.
void get_Acc_Fifo_Avg(int32_t* X, int32_t* Y, int32_t* Z);

#endif
