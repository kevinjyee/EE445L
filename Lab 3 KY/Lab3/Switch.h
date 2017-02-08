// Switch.h
// This software configures the off-board Switch keys
// Runs on LM4F120 or TM4C123

// Lab number: 6
// Hardware connections

// Header files contain the prototypes for public functions
// this file explains what the module does

// **************Switch_Init*********************
// Initialize Switch key inputs, called once 
// Input: none 
// Output: none
void Switch_Init(void);

// **************Switch_In*********************
// Input from Switch key inputs 
// Input: none 
// Output: 0 to 7 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
uint32_t Switch_In(void);

void Timer2Arm(void);

void init_switchmain(void);