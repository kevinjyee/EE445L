#include <stdint.h>
#include "inc//tm4c123gh6pm.h"
#include "FIFOqueue.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

 volatile unsigned long LastA;      // previous input for port A
volatile unsigned long LastF;      // previous input for port F 

uint16_t timeout_Count = 0;
uint32_t Switch_In(void);


#define RELOAD_10HZ	0x4C4B40 // Reload value for an interrupt frequency of 10Hz.
#define BUTTONAPRESS 0x20

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

static void GPIOArm_PortF(void){
  GPIO_PORTF_ICR_R = 0x1F;      // (e) clear flags
  GPIO_PORTF_IM_R |= 0x1F;      // (f) arm interrupt on PA3-0 *** No IME bit as mentioned in Book ***
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFFF1F)|0x000000A0; //priority 5
	NVIC_EN0_R = SYSCTL_RCGC2_GPIOF;
}


// **************GPIOArm*********************
// Initialize switch key inputs, called once 
// Input: none 
// Output: none

static void GPIOArm_PortA(void){
  GPIO_PORTA_ICR_R = 0x01;      // (e) clear flags
  GPIO_PORTA_IM_R |= 0x01;      // (f) arm interrupt on PA3-0 *** No IME bit as mentioned in Book ***
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFFF1F)|0x000000A0; //priority 5
	NVIC_EN0_R = SYSCTL_RCGC2_GPIOA;
}


// **************Switch_Init*********************
// Initialize switch key inputs, called once 
// Input: none 
// Output: none
void PortF_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // activate port F
	while((SYSCTL_PRGPIO_R&0x10)==0){};
  GPIO_PORTF_DIR_R &=~ 0x1F;      // make PF4-0 in make the input pins
  GPIO_PORTF_AFSEL_R &= ~0x1F;   // disable alt funct on PF4-0
	GPIO_PORTF_AMSEL_R &= ~0xFF;      // no analog on PF4-0
  GPIO_PORTF_PCTL_R &= ~0xFFFFFFFF; // regular function
  GPIO_PORTF_DEN_R |= 0x1F;      // enable digital I/O on PF4-0
	GPIO_PORTF_IS_R &= ~0x1F;         // 8) edge-sensitive
  GPIO_PORTF_IBE_R |= 0x1F;        // 9) both edges
	
}

void PortA_Init(void){
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A
	while((SYSCTL_PRGPIO_R&0x01)==0){};
  GPIO_PORTA_DIR_R &=~ 0x01;      // make PF4-0 in make the input pins
  GPIO_PORTA_AFSEL_R &= ~0x01;   // disable alt funct on PA0
	GPIO_PORTA_AMSEL_R &= ~0x01;      // no analog on PA0
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFF0FFFF)+0x00000000; //Port 0 Regulat function. Everything else LCD
  GPIO_PORTA_DEN_R |= 0x01;      // enable digital I/O on PA0
	GPIO_PORTA_IS_R &= ~0x01;         // 8) edge-sensitive
  GPIO_PORTA_IBE_R |= 0x01;        // 9) both edges
}


// **************Switch_In*********************
// Input from Switch key inputs 
// Input: none 
// Output: 1 to 3 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
uint32_t Switch_In(void){
	uint32_t inputs;
	inputs = (0x1F & GPIO_PORTF_DATA_R);		//get inputs of switches
  return inputs;
}


// **************PortA_Handler********************
// Input from Switch key inputs 
// Input: none 
// Output: 1 to 3 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
void GPIOPortA_Handler(void)
{
	GPIO_PORTA_IM_R &= ~0x01; // disarm interrupt on PA so we dont get double clicks
	long i_bit = StartCritical();
	timeout_Count = 0;
	EndCritical(i_bit);
	if(LastA){    // 0x0F means it was previously released
 
		Fifo_Put(BUTTONAPRESS); //Register the falling edge only.
  }
	Timer2Arm(); //arm the timer again to be ready for countdown

}

// **************PortF_Handler********************
// Input from Switch key inputs 
// Input: none 
// Output: 1 to 3 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
void GPIOPortF_Handler(void)
{
	GPIO_PORTF_IM_R &= ~0x0F; // disarm interrupt on PA so we dont get double clicks
	long i_bit = StartCritical();
	timeout_Count = 0;
	EndCritical(i_bit);
	if(LastF){    // 0x0F means it was previously released
 
		Fifo_Put(LastF); //Register the falling edge only.
  }
	Timer2Arm(); //arm the timer again to be ready for countdown

}


void Timer2A_Handler(void){
  TIMER2_IMR_R = 0x00000000;    // disarm timeout interrupt
	LastA = (0x01 & GPIO_PORTA_DATA_R); //get inputs of switches from Port A 
	LastF = (0x1F & GPIO_PORTF_DATA_R);		//get inputs of switches  // switch state
	GPIOArm_PortF(); //Timer is done, so arm the handler to listen
	GPIOArm_PortA();
}

void Switch_Init(void){
	Fifo_Init(); 
	PortF_Init();
	Timer2Arm();
}
