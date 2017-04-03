#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Timer4.h"
#include "SysTick.h"


volatile int steps =0;
volatile int prevsteps = 0;
extern volatile int stepsper2sec;
extern volatile int Timer5Time;
extern volatile int BPM;
extern volatile int last_time;
extern volatile int total_time;
extern volatile int BPMsteps;
// ***************** Timer5_Init ****************
// Activate Timer5 to count time for tasks
// Inputs:  Period in units (1/clockfreq)
// Outputs: none
void Timer5_Init(uint32_t period) {
    SYSCTL_RCGCTIMER_R |= 0x20;                 // 0) activate timer5
     while((SYSCTL_PRTIMER_R&0x20) == 0) {};    // allow time for clock to stabilize
    TIMER5_CTL_R = 0x00000000;                  // 1) disable timer5A during setup
    TIMER5_CFG_R = 0x00000000;                  // 2) configure for 32-bit mode
    TIMER5_TAMR_R = 0x00000002;                 // 3) configure for periodic mode, default down-count settings
    TIMER5_TAILR_R = period - 1;                // 4) reload value
    TIMER5_TAPR_R = 0;                          // 5) bus clock resolution
    TIMER5_ICR_R = 0x00000001;                  // 6) clear timer5A timeout flag
    TIMER5_IMR_R = 0x00000001;                  // 7) arm timeout interrupt
    NVIC_PRI23_R &= 0xFFFFFF00;                 // 8) priority 0 (bits 7,6,5)
    NVIC_PRI23_R |= 0x00000070;                 // 8) priority 0 TODO: Make it so we're only touching the three priority bits :/
    NVIC_EN2_R = 1<<(92-(32*2));                          //  enable IRQ 92 in NVIC
    TIMER5_CTL_R = 0x00000001;                            //  enable timer5A
		prevsteps = steps;
}

void Timer5A_Handler(void) {
    TIMER5_ICR_R = TIMER_ICR_TATOCINT;          // acknowledge TIMER5A timeout
		stepsper2sec = steps*256 - prevsteps*256;
		if(stepsper2sec ==0)
		{
			BPM = 0;
			last_time = 0;
			total_time = 0;
			BPMsteps = 0;
		}
		prevsteps = steps;
		Timer5Time += 2;
}

