#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Timer4.h"

void (*T4PeriodicTask)(void);   // user function

extern volatile int Timer4Time;

// ***************** Timer4_Init ****************
// Inputs:  period in units (1/clockfreq)
// Outputs: none
void Timer4_Init(void(*task)(void),unsigned long period){
  SYSCTL_RCGCTIMER_R |= 0x10;                         // 0) activate TIMER4
	while((SYSCTL_PRTIMER_R&0x10) == 0){};              // allow time for clock to stabilize
	T4PeriodicTask = task;
  TIMER4_CTL_R = 0x00000000;                          // 1) disable TIMER4A during setup
  TIMER4_CFG_R = 0x00000000;                          // 2) configure for 32-bit mode
  TIMER4_TAMR_R = 0x00000002;                         // 3) configure for periodic mode, default down-count settings
  TIMER4_TAILR_R = period-1;                          // 4) reload value
  TIMER4_TAPR_R = 0;                                  // 5) bus clock resolution
  TIMER4_ICR_R = 0x00000001;                          // 6) clear TIMER4A timeout flag
  TIMER4_IMR_R = 0x00000001;                          // 7) arm timeout interrupt
	NVIC_PRI17_R &= 0xFF00FFFF;                         // 8) priority 7 
		NVIC_PRI17_R |= 0x00FF0000;                         // 8) priority 7 TODO: Make it so we're only touching the three priority bits :/
	NVIC_EN2_R = 1<<(70-(32*2));       									// enable IRQ 70 in NVIC
	TIMER4_CTL_R = 0x00000001;         									// enable TIMER4A
}

// ***************** Timer4A_Handler ******************
// Timer4 is in charge of running background threads in the OS.
void Timer4A_Handler(void){
  TIMER4_ICR_R = TIMER_ICR_TATOCINT;                  // acknowledge TIMER4A timeout
	(*T4PeriodicTask)();   
	Timer4Time+=1;
}