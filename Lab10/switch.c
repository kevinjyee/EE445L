#include <stdint.h>
#include "inc//tm4c123gh6pm.h"
#include "FIFO.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint16_t timeout_Count = 0;
uint32_t Switch_In(void);
volatile uint32_t LastE = 0;

#define RELOAD_10HZ	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.

// ***************** Timer2_ARM ****************
// Activate Timer2 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer2Arm(void){
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  //PeriodicTask = task;          // user function
  TIMER2_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000001;   // 3) OneSHot Mode
  TIMER2_TAILR_R = 160000;      // 4) 10ms reload value (period) 
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution (prescale register) Timer frequency will be bus frequency divided by prescale + 1 
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt (timer register is requested) 
	NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4 (change to priority 4, lower priority than timer 0)
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
}

// **************GPIOArm*********************
// Initialize switch key inputs, called once 
// Input: none 
// Output: none

static void GPIOArm(void){
  GPIO_PORTE_ICR_R = 0x0F;      // (e) clear flags
  GPIO_PORTE_IM_R |= 0x0F;      // (f) arm interrupt on PA3-0 *** No IME bit as mentioned in Book ***
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFFF1F)|0x000000A0; //priority 5
	NVIC_EN0_R = SYSCTL_RCGC2_GPIOE;
}



// **************Switch_Init*********************
// Initialize switch key inputs, called once 
// Input: none 
// Output: none
void Switch_Input_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; // activate port E
	while((SYSCTL_PRGPIO_R&0x10)==0){};
  GPIO_PORTE_DIR_R &=~ 0x0C;      // make PE3-2 in make the input pins
  GPIO_PORTE_AFSEL_R &= ~0x0C;   // disable alt funct on PA3-0
	GPIO_PORTE_AMSEL_R &= ~0x0C;      // no analog on PA3-0
  GPIO_PORTE_PCTL_R &= ~0x0000000C; // regular function
  GPIO_PORTE_DEN_R |= 0x0C;      // enable digital I/O on PA3-0
	GPIO_PORTE_IS_R &= ~0x0C;         // 8) edge-sensitive
  GPIO_PORTE_IBE_R |= 0x0C;        // 9) both edges
	
}

// **************Switch_In*********************
// Input from Switch key inputs 
// Input: none 
// Output: 0, 1, 2, or 3 depending on keys
// 0x1 is just Key0, 0x2 is just Key1
uint32_t Switch_In(void){
	uint32_t inputs;
	inputs = (0x0C & GPIO_PORTE_DATA_R);		//get inputs of switches
  return inputs >> 2;
}


void GPIOPortE_Handler(void)
{
	GPIO_PORTE_IM_R &= ~0x0F; // disarm interrupt on PA so we dont get double clicks
	long i_bit = StartCritical();
	timeout_Count = 0;
	EndCritical(i_bit);
	if(LastE){    // 0x0F means it was previously released
 
		Fifo_Put(LastE); //Register the falling edge only.
  }
	Timer2Arm(); //arm the timer again to be ready for countdown

}


void Timer2A_Handler(void){
  TIMER2_IMR_R = 0x00000000;    // disarm timeout interrupt
	LastE = Switch_In();  // switch state
	GPIOArm(); //Timer is done, so arm the handler to listen
}

// **************Switch_Init*********************
// Initialize Switch key inputs and FIFO data collection. Called once.
// Input: none 
// Output: none
void Switch_Init(void){
	Fifo_Init(); 
	Switch_Input_Init();
	Timer2Arm();
	//Timer1A_Init();
}
