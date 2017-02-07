#include <stdint.h>
#include "inc//tm4c123gh6pm.h"
#include "FIFO.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


volatile static unsigned long LastA;      // previous input for port A 
uint32_t Switch_In(void);



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
   TIMER0_TAILR_R = 160000;      // 4) 10ms reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4 (change to priority 4, lower priority than timer 0)
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
  GPIO_PORTA_ICR_R = 0x0F;      // (e) clear flags
  GPIO_PORTA_IM_R |= 0x0F;      // (f) arm interrupt on PA3-0 *** No IME bit as mentioned in Book ***
  NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFFFFF1F)|0x000000A0; //priority 5
	NVIC_EN0_R = 1; 
}



// **************Switch_Init*********************
// Initialize switch key inputs, called once 
// Input: none 
// Output: none
void Switch_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A	
	while((SYSCTL_PRGPIO_R&0x01)==0){};
  GPIO_PORTA_DIR_R &=~ 0x0F;      // make PA3-0 in
  GPIO_PORTA_AFSEL_R &= ~0x0F;   // disable alt funct on PA3-0
	GPIO_PORTA_AMSEL_R &= ~0x0F;      // no analog on PA3-0
  GPIO_PORTA_PCTL_R &= ~0x0000FFFF; // regular function
  GPIO_PORTA_DEN_R |= 0x0F;      // enable digital I/O on PA3-0
	GPIOArm();
}

// **************Switch_In*********************
// Input from Swithc key inputs 
// Input: none 
// Output: 1 to 3 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
uint32_t Switch_In(void){
	uint32_t inputs;
	inputs = (0x0F & GPIO_PORTA_DATA_R);		//get inputs of switches
  return inputs;
}


void GPIOPortA_Handler(void)
{
	GPIO_PORTA_IM_R &= ~0x0F;     // disarm interrupt on PA
	if(LastA){    // 0x0F means it was previously released
 
		RxFifo_Put(Switch_In());
  }

}


void Timer2A_Handler(void){
  TIMER2_IMR_R = 0x00000000;    // disarm timeout interrupt
	LastA = Switch_In();  // switch state
	GPIOArm();
}
