#include <stdint.h>
#include "FIFO.h"

#define FIFO_SIZE 32
uint32_t static *Putpt; //Points to the location where the next element to be added goes
uint32_t  *Getpt; // Points to the location of the oldest valid element, hance the elements to be removed first
uint32_t static Fifo[FIFO_SIZE];

// Initialize FIFO (obtained from book)
void Fifo_Init(void){
	long sr;
	sr = StartCritical();
	Putpt = Getpt = &Fifo[0]; // puts Putpt and Getpt on the same address
	EndCritical(sr);
}



//------------------FIFO PUT------------------

/*FIFO_Put
1)stores a single value on the FIFO queue
2)operates with interrupts disabled
3)updates PutI
4)detects buffer full condition
5)handles transition from LIMIT-1 to FIRST
*/

int Fifo_Put(uint32_t data){
	uint32_t *TempPt;
	//Checks if there is space.
	TempPt = Putpt + 1;
	
	//If the Put pointer reaches the bottom of the array, it wraps around to the beginning of the array.
	if(TempPt==&Fifo[FIFO_SIZE]){
		TempPt = &Fifo[0];
	}
	if(TempPt == Getpt){
		return 0; //The queue is full.
	}
	else{
		*(Putpt) = data; //Save data into an array named PutPt
		Putpt = TempPt;  //OK Updates the Address of Putpt
		return 1;
	}
}
//---------------FIFO_GET-------------------------
/*
1)reads a single value from the FIFO queue
2)operates with interrupts disabled
3)updates GetI
4)detects buffer empty condition
5)handles transition from LIMIT-1 to FIRST
*/
int Fifo_Get(uint32_t *datapt){
	if(Putpt == Getpt){
		return 0; //The queue is empty
	}
	else{
		*datapt = *(Getpt++); //Adds 4 bytes, 32 bit message
		//Wrap Around
		if(Getpt == &Fifo[FIFO_SIZE]){
			Getpt = &Fifo[0];
		}
		return 1;
	}
}
