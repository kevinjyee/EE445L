/* File Name:    DAC.c
 * Authors:      Kevin Yee (kjy252), Stefan Bordovsky (sb39782)
 * Created:      May 5, 2015 by Daniel Valvano
 * Description:  This program serves as an interface with the DAC in order to
 * 		play music on command.
 *               
 * 
 * Lab Number: MW 3:30-5:00
 * TA: Mahesh
 * Last Revised: 2/08/2017	
 */
 
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../SysTick.h"

// ***************** DAC_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void DAC_Init(uint16_t data){
	SYSCTL_RCGCSSI_R |= 0x02;							// Activate SSI1
	SYSCTL_RCGCGPIO_R != 0x08;						// Activate port D
	while((SYSCTL_PRGPIO_R&0x08) == 0){};	// Wait until ready.
	GPIO_PORTD_AFSEL_R |= 0x0B;						// Enable alt. function on PD0, 1, 3
	GPIO_PORTD_DEN_R |= 0x0B;							// Configure PD2, 3, 5 as SSI
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0F00) + 0x00002022; // Write 2 to port to signify SSI.
	GPIO_PORTD_AMSEL_R = 0;								// Disable analog functionality on Port D
	SSI1_CR1_R = 0x00000000;							// Disable SSI, master mode
	SSI1_CPSR_R = 0x02;										// 8MHz SSIClk
	SSI1_CR0_R &= ~(0x0000FFF0);					// SCR = 0, SPH = 0, Freescale
	SSI1_CR0_R |= SSI_CR0_SPO;						// SPO = 0 (flip clock bit)
	SSI1_CR0_R |= 0x0F;										// DSS = 16-bit data
	SSI1_DR_R = data;											// Load data into TX FIFO
	SSI1_CR1_R |= 0x00000002;							// Enable SSI
}

// ***************** DAC_Out ****************
// Outputs a voltage value to the DAC
// Inputs:  none
// Outputs: none
void DAC_Out(){
	while((SSI1_SR_R & 0x00000002) == 0){}; // SSI TX FIFO not full.
	SSI0_DR_R = code;
}