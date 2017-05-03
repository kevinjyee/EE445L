/* File Name:    DAC.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program serves as an interface with the DAC in order to
 * 		play music on command.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 4/05/2017	
 */
 
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"
#include "DAC.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value


// ***************** DAC_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void DAC_Init(uint16_t data){
	SYSCTL_RCGCSSI_R |= 0x02;							// Activate SSI1
	SYSCTL_RCGCGPIO_R |= 0x08;						// Activate port D
	while((SYSCTL_PRGPIO_R&0x08) == 0){};	// Wait until ready.
	GPIO_PORTD_AFSEL_R |= 0x0B;						// Enable alt. function on PD0, 1, 3
	GPIO_PORTD_DEN_R |= 0x0B;							// Configure PD0, 1, 3 as SSI
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0F00) + 0x00002022; // Write 2 to port to signify SSI.
	GPIO_PORTD_AMSEL_R = 0;								// Disable analog functionality on Port D
	SSI1_CR1_R = 0x00000000;							// Disable SSI, master mode
	SSI1_CPSR_R = 0x02;										// fSSI = fBUS/(CPSDVSR  ( 1 + SCR) = fSSI = 50MHz/4 = 12MHz SSIClk
	SSI1_CR0_R &= ~(0x0000FFF0);					// SCR = 0, SPH = 0, Freescale
	SSI1_CR0_R |= SSI_CR0_SPO;						// SPO = 1 (flip clock bit)
	SSI1_CR0_R |= 0x0F;										// DSS = 16-bit data
	//SSI1_DR_R = data;											// Load data into TX FIFO

	SSI1_DR_R = (data & 0x0FFF) + (3 << 14); // Update DAC value for output A and update output B from buffer.
		
	SSI1_CR1_R |= 0x00000002;							// Enable SSI
}

#define PB457   (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7)
// ***************** DAC_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void DAC_InitB(uint16_t data){
	SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R2;							// Activate SSI2
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;						// Activate port B
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R1) == 0){};	// Wait until port B is ready
	GPIO_PORTB_AFSEL_R |= 0xB0;						// Enable alt. function on PB4, 5, 7
	GPIO_PORTB_DEN_R |= 0xB0;							// Configure PD0, 1, 3 as SSI
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x0F00FFFF) + 0x20220000; // Write 2 to port to signify SSI.
	GPIO_PORTB_AMSEL_R  &= ~0xB0;								// Disable analog functionality on Port D
	SSI2_CR1_R = 0x00000000;							// Disable SSI, master mode
	SSI2_CPSR_R = 0x02;										// fSSI = fBUS/(CPSDVSR  ( 1 + SCR) = fSSI = 50MHz/4 = 12MHz SSIClk
	SSI2_CR0_R &= ~(SSI_CR0_SCR_M | SSI_CR0_SPH | SSI_CR0_FRF_M);
  SSI2_CR0_R |= SSI_CR0_SPO | SSI_CR0_FRF_MOTO | SSI2_CR0_R | SSI_CR0_DSS_16;
	SSI2_CR0_R |= 0x0F;										// DSS = 16-bit data
	//SSI2_DR_R = data;											// Load data into TX FIFO

	SSI2_DR_R = (data & 0x0FFF) + (3 << 14); // Update DAC value for output A and update output B from buffer.
		
	SSI2_CR1_R |= SSI_CR1_SSE;							// Enable SSI
}

// ***************** DAC_Out ****************
// Outputs a voltage value to the DAC
// Inputs:  none
// Outputs: none
void DAC_Out(uint16_t code){
	DisableInterrupts();
	while((SSI1_SR_R & 0x00000002) == 0){}; // SSI TX FIFO not full.
		// Below = single channel, TLV 5616
	  // SSI1_DR_R = code;
		// Below = dual channel
		 SSI1_DR_R = (code & 0x0FFF) + (1 << 12);
		 SSI1_DR_R = (code & 0x0FFF) + (1 << 15);
		//SSI1_DR_R = (code & 0x0FFF) + (3 << 14); 
	while((SSI2_SR_R & SSI_SR_TNF)==0){};
		//SSI2_DR_R = code;   
		//SSI2_DR_R = (code & 0x0FFF) + (3 << 14); 
		 SSI2_DR_R = (code & 0x0FFF) + (1 << 12);
		 SSI2_DR_R = (code & 0x0FFF) + (1 << 15);
		EnableInterrupts();
	//SSI1_DR_R = (code & 0x0FFF) + (3 << 14); // Write value for output B to buffer.
}


//****************Delay**********************

void DelayWait5ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/182;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}
//****************Simple_DAC_Test************
void Simple_DAC_Test()
{
	uint32_t sampleNotes[8] = {0,64,128,256,512,1024,1536,2048};
	for(int i =0; i < 8; i++)
	{
		DAC_Out(sampleNotes[i]);
		DelayWait5ms(1);
	}
}
