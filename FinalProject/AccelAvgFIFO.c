#include <stdint.h>
#include "AccelAvgFIFO.h"
#include <math.h>
#include "Timer5.h"
#include "Accelerometer.h"

#define ACC_FIFO_SIZE 4
#define NEW_FIXED 0
#define OLD_FIXED 1
#define TRUE 1
#define FALSE 0
#define FIVE_HZ_RELOAD 0xF42400
#define THREE_PT_THREE_HZ_RELOAD 0x16E3600

uint32_t static *XPutAcc; //Points to the location where the next element to be added goes
uint32_t static *YPutAcc; //Points to the location where the next element to be added goes
uint32_t static *ZPutAcc; //Points to the location where the next element to be added goes
uint32_t static XAccFifo[ACC_FIFO_SIZE];
uint32_t static YAccFifo[ACC_FIFO_SIZE];
uint32_t static ZAccFifo[ACC_FIFO_SIZE];
uint32_t static XLinShiftR[2]; // [0] = sample_new, [1] = sample_old.
uint32_t static YLinShiftR[2]; // [0] = sample_new, [1] = sample_old.
uint32_t static ZLinShiftR[2]; // [0] = sample_new, [1] = sample_old.
uint32_t volatile XAvg = 0; // Fixed point average value, resolution = 0.01
uint32_t volatile YAvg = 0; // Fixed point average value, resolution = 0.01
uint32_t volatile ZAvg = 0; // Fixed point average value, resolution = 0.01
uint32_t static XMin; // Min x value in last 50 samples.
uint32_t static XMax; // Max x value in last 50 samples.
uint32_t static YMin; // Min y value in last 50 samples.
uint32_t static YMax; // Max y value in last 50 samples.
uint32_t static ZMin; // Min z value in last 50 samples.
uint32_t static ZMax; // Max z value in last 50 samples.
uint32_t volatile XThresh = 0; // Dynamic X threshold.
uint32_t volatile YThresh = 0; // Dynamic Y threshold.
uint32_t volatile ZThresh = 0; // Dynamic Z threshold.
uint8_t static numSamples = 0;
uint32_t static count = 0;  // Keep track of num elements in FIFO.
uint16_t precision = 150;
volatile uint32_t stepCount = 0;

uint8_t volatile DetectionAxis = 0;

// Implementation of http://www.analog.com/media/en/analog-dialogue/volume-44/number-2/articles/pedometer-design-3-axis-digital-acceler.pdf


// Reset max and min values.
void reset_Local_Extrema(){
	XMax = 0;
	YMax = 0;
	ZMax = 0;
	XMin = 4097;
	YMin = 4097;
	ZMin = 4097;
}

void update_Local_Extrema(){
	if(XAvg > XMax){
		XMax = XAvg;
	}
	if(XAvg < XMin){
		XMin = XAvg;
	}
	if(YAvg > YMax){
		YMax = YAvg;
	}
	if(YAvg < YMin){
		YMin = YAvg;
	}
	if(ZAvg > ZMax){
		ZMax = ZAvg;
	}
	if(ZAvg < ZMin){
		ZMin = ZAvg;
	}
}

void calculate_Thresholds(){
	XThresh = (XMax + XMin) / 2;
	YThresh = (YMax + YMin) / 2;
	ZThresh = (ZMax + ZMin) / 2;
}

uint8_t linear_Shift(){
	XLinShiftR[OLD_FIXED] = XLinShiftR[NEW_FIXED]; // sample_old <- sample_new
	YLinShiftR[OLD_FIXED] = YLinShiftR[NEW_FIXED];
	ZLinShiftR[OLD_FIXED] = ZLinShiftR[NEW_FIXED];
	int32_t xDiff = XAvg - XLinShiftR[NEW_FIXED];
	if(xDiff < 0){
		xDiff *= -1;
	}
	if(xDiff > precision){
		XLinShiftR[NEW_FIXED] = XAvg;
	}
	int32_t yDiff = YAvg - YLinShiftR[NEW_FIXED];
	if(yDiff < 0){
		yDiff *= -1;
	}
	if(yDiff > precision){
		YLinShiftR[NEW_FIXED] = YAvg;
	}
	int32_t zDiff = ZAvg - ZLinShiftR[NEW_FIXED];
	if(zDiff < 0){
		zDiff *= -1;
	}
	if(zDiff > precision){
		ZLinShiftR[NEW_FIXED] = ZAvg;
	}
	xDiff = XLinShiftR[OLD_FIXED] - XLinShiftR[NEW_FIXED];
	if(xDiff < 0){
		xDiff *= -1;
	}
	yDiff = YLinShiftR[OLD_FIXED] - YLinShiftR[NEW_FIXED];
	if(yDiff < 0){
		yDiff *= -1;
	}
	zDiff = ZLinShiftR[OLD_FIXED] - ZLinShiftR[NEW_FIXED];
	if(zDiff < 0){
		zDiff *= -1;
	}
	/*
	xDiff = fabs((int32_t) XLinShiftR[1] - (int32_t) XLinShiftR[0]);
	yDiff = fabs((int32_t) YLinShiftR[1] - (int32_t) YLinShiftR[0]);
	zDiff = fabs((int32_t) ZLinShiftR[1] - (int32_t) ZLinShiftR[0]);
	*/
	if((xDiff >= yDiff) && (xDiff >= zDiff)){
		if((XLinShiftR[OLD_FIXED] > XThresh) && (XLinShiftR[NEW_FIXED] < XThresh) && !NextStepTooSoon){
			stepCount++;
			DetectionAxis = 1;
			NextStepTooSoon = TRUE;
			Timer5_Init(THREE_PT_THREE_HZ_RELOAD);
			return 1;
		}
	} else if((yDiff >= xDiff) && (yDiff >= zDiff)){
		if((YLinShiftR[OLD_FIXED] > YThresh) && (YLinShiftR[NEW_FIXED] < YThresh) && !NextStepTooSoon){
			stepCount++;
			DetectionAxis = 2;
			NextStepTooSoon = TRUE;
			Timer5_Init(THREE_PT_THREE_HZ_RELOAD);
			return 1;
		}
	} else{
		if((ZLinShiftR[OLD_FIXED] > ZThresh) && (ZLinShiftR[NEW_FIXED] < ZThresh) && !NextStepTooSoon){
			stepCount++;
			DetectionAxis = 3;
			NextStepTooSoon = TRUE;
			Timer5_Init(THREE_PT_THREE_HZ_RELOAD);
			return 1;
		}
	}
	/* // Legacy code. Was probably causing bug.
	if((xDiff > XThresh) || (yDiff > YThresh) || (zDiff > ZThresh)){
		stepCount++;
		return 1;
	}
	*/
	return 0;
}

// Initialize FIFO (obtained from book)
void Acc_Fifo_Init(void){
	long sr;
	sr = StartCritical();
	XPutAcc = &XAccFifo[0]; // puts Putpt and Getpt on the same address
	YPutAcc = &YAccFifo[0]; // puts Putpt and Getpt on the same address
	ZPutAcc = &ZAccFifo[0]; // puts Putpt and Getpt on the same address
	for(int i = 0; i < ACC_FIFO_SIZE; i++){
		XAccFifo[i] = 0;
		YAccFifo[i] = 0;
		ZAccFifo[i] = 0;
	}
	XAvg = 0;
	YAvg = 0;
	ZAvg = 0;
	XLinShiftR[0] = 0;
	XLinShiftR[1] = 0;
	YLinShiftR[0] = 0;
	YLinShiftR[1] = 0;
	ZLinShiftR[0] = 0;
	ZLinShiftR[1] = 0;
	reset_Local_Extrema();
	EndCritical(sr);
}

//------------------ACC FIFO PUT------------------

/*Acc_FIFO_Put
1)stores a single value on the FIFO queue
2)operates with interrupts disabled
3)updates PutI
4)detects buffer full condition
5)handles transition from LIMIT-1 to FIRST
*/

uint8_t Acc_Fifo_Put(uint32_t X, uint32_t Y, uint32_t Z){
	uint8_t stepCheck = 0;
	uint32_t *TempXPt;
	uint32_t *TempYPt;
	uint32_t *TempZPt;
	int old_x_data;
	int old_y_data;
	int old_z_data;
	//Checks if there is space.
	TempXPt = XPutAcc + 1;
	TempYPt = YPutAcc + 1;
	TempZPt = ZPutAcc + 1;
	if(count < ACC_FIFO_SIZE - 1){ // If still fewer than ACC_FIFO_SIZE elements, increment count.
		NextStepTooSoon = 0;
		count++;
	}
	numSamples = (numSamples + 1) % 50;
	if(numSamples == 0){
		calculate_Thresholds();
		reset_Local_Extrema();
	}
	//If the Put pointer reaches the bottom of the array, it wraps around to the beginning of the array.
	if(TempXPt==&XAccFifo[ACC_FIFO_SIZE]){
		TempXPt = &XAccFifo[0];
		TempYPt = &YAccFifo[0];
		TempZPt = &ZAccFifo[0];
	}
	old_x_data = *(XPutAcc);
	old_y_data = *(YPutAcc);
	old_z_data = *(ZPutAcc);
	if(old_x_data){ // Acc Average queue full.
		XAvg -= (old_x_data) / ACC_FIFO_SIZE;
		XAvg += (X) / ACC_FIFO_SIZE;
		*(XPutAcc) = X;
		XPutAcc = TempXPt;
		YAvg -= (old_y_data) / ACC_FIFO_SIZE;
		YAvg += (Y) / ACC_FIFO_SIZE;
		*(YPutAcc) = Y;
		YPutAcc = TempYPt;
		ZAvg -= (old_z_data) / ACC_FIFO_SIZE;
		ZAvg += (Z) / ACC_FIFO_SIZE;
		update_Local_Extrema();
		*(ZPutAcc) = Z;
		ZPutAcc = TempZPt;
		stepCheck = linear_Shift(); // 1 if measured step within linear_shift(), 0 otherwise.
	} else{ // Queue not full yet.
		XAvg += (X) / ACC_FIFO_SIZE;
		*(XPutAcc) = X;
		XPutAcc = TempXPt;
		YAvg += (Y) / ACC_FIFO_SIZE;
		*(YPutAcc) = Y;
		YPutAcc = TempYPt;
		ZAvg += (Z) / ACC_FIFO_SIZE;
		*(ZPutAcc) = Z;
		ZPutAcc = TempZPt;
	}
	return stepCheck;
}

// *****************get_Acc_Fifo_Avg ****************
// Gets the rolling acceleration average
// Inputs:  None
// Outputs: -1 if average not fully computed, otherwise = avg.
void get_Acc_Fifo_Avg(int32_t* X, int32_t* Y, int32_t* Z){
	if(count == ACC_FIFO_SIZE){ // Return avg if all FIFO spots filled, else return -1.
		*X = XAvg;
		*Y = YAvg;
		*Z = ZAvg;
	} else{
		*X = -1;
		*Y = -1;
		*Z = -1;
	}
}