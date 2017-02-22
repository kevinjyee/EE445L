#include <stdint.h>
#include "inc//tm4c123gh6pm.h"
#include "FIFOqueue.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


extern volatile unsigned long LastE;      // previous input for port A 

uint16_t timeout_Count = 0;
uint32_t Switch_In(void);

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

/*
// ***************** Timer1A_Init ****************
// Activate Timer1 interrupts to send user back to main menu after 10
// 		seconds of inactivity.
// Inputs:  None
// Outputs: none
void Timer1A_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  //PeriodicTask = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = RELOAD_10HZ;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00010000; // 8) priority 6
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}


// ***************** Timer1A_Handler ****************
// Count number of 10Hz interrupts. After counting 10s worth of time
//	between button presses, put menu switch input into FIFO to send
//	user back to main screen.
// Inputs:  None
// Outputs: none
void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
	long i_bit = StartCritical();
	timeout_Count = (timeout_Count + 1) % 100;
	if(timeout_Count == 99){ //
		Fifo_Put(0x08); //Register the falling edge only.
	}
	EndCritical(i_bit);
}

// ***************** Disable_Timer1 ****************
// Disables Timer1 to stop screen timeout during an alarm.
// Inputs:  none
// Outputs: none
void Disable_Timer1(void){
	TIMER1_CTL_R = 0x00000000;
}

// ***************** Enable_Timer1 ****************
// Enables Timer1 to start screen timeout counter.
// Inputs:  none
// Outputs: none
void Enable_Timer1(void){
	TIMER1_CTL_R = 0x00000001;   
}
*/

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
void Switch_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; // activate port E
	while((SYSCTL_PRGPIO_R&0x10)==0){};
  GPIO_PORTE_DIR_R &=~ 0x0F;      // make PA3-0 in make the input pins
  GPIO_PORTE_AFSEL_R &= ~0x0F;   // disable alt funct on PA3-0
	GPIO_PORTE_AMSEL_R &= ~0x0F;      // no analog on PA3-0
  GPIO_PORTE_PCTL_R &= ~0xFFFFFFFF; // regular function
  GPIO_PORTE_DEN_R |= 0x0F;      // enable digital I/O on PA3-0
	GPIO_PORTE_IS_R &= ~0x0F;         // 8) edge-sensitive
  GPIO_PORTE_IBE_R |= 0x0F;        // 9) both edges
	
}

// **************Switch_In*********************
// Input from Switch key inputs 
// Input: none 
// Output: 1 to 3 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
uint32_t Switch_In(void){
	uint32_t inputs;
	inputs = (0x0F & GPIO_PORTE_DATA_R);		//get inputs of switches
  return inputs;
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

void init_switchmain(void){
	Fifo_Init(); 
	Switch_Init();
	Timer2Arm();
	//Timer1A_Init();
}
