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

void Accel_Test(void);