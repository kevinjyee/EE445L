// FIFOqueue.h
// Runs on LM4F120/TM4C123
// Allow access to a FIFOqueue for storing input history from switches.
//	Supports Get, Put, and Length queries.
// Stefan Bordovsky and Kevin Yee
// February 16, 2016

#ifndef __queue_h__
#define __queue_h__
#include "inc/tm4c123gh6pm.h"


long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

// ***************** Fifo_Init ****************
// Initializes the Fifo Queue.
// Inputs:  none
// Outputs: none
void Fifo_Init(void);

// ***************** Fifo_Put ****************
// Puts values into the FIFO queue.
// Inputs:  32-bit data.
// Outputs: Returns 1 if data successfully put, 0 otherwise.
int Fifo_Put(uint32_t data);

// ***************** Fifo_Get ****************
// Removes entries from the queue in the order that they were put in.
// Inputs:  Pointer to an integer which will be loaded with data.
// Outputs: Returns 1 if successful, 0 otherwise.
int Fifo_Get(uint32_t *datapt);

// ***************** Fifo_Get ****************
// Returns length of the FIFO buffer.
// Inputs:  none
// Outputs: The size of the FIFO queue.
unsigned short Fifo_Length(void);

#endif
