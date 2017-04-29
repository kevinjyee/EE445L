// Timer4.h
// Runs on TM4C123 
// Use Timer4 in 32-bit periodic mode to request interrupts at a periodic rate


// ***************** Timer4_Init ****************
void Timer4_Init(void(*task)(void),unsigned long period);


// ***************Timer4A_Handler ************
void Timer4A_Handler(void);
