#ifndef __queue_h__
#define __queue_h__
#include "inc/tm4c123gh6pm.h"


long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

//Initializes the Fifo Queue.
void Fifo_Init(void);
//Puts values into the queue.
//Returns 1 if successful.
int Fifo_Put(uint32_t data);
//Removes entries from the queue in the order that they were put in.
//Returns 1 if successful.
int Fifo_Get(uint32_t *datapt);
//Obtains the size of the fifo queue.
unsigned short Fifo_Length(void);

#endif
