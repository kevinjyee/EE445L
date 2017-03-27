/* File Name:    DAC8551Test.c
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
#include "SysTick.h"
#include "DAC.h"

#define SSI1_CR1_EOT 0x0002;


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

// ***************** SSI1_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void SSI1_Init(){
	GPIO_PORTD_DIR_R |= 0x02;   // make PD1 out (FS output)
  GPIO_PORTD_AFSEL_R &= ~0x04;// disable alt funct on PF2
	GPIO_PORTD_AFSEL_R |= 0x09;						// Enable alt. function on PD0, 3
	GPIO_PORTD_DEN_R |= 0x0B;							// Configure PD0, 1, 3 as SSI
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0F00) + 0x00002002; // Write 2 to port to signify SSI.
	GPIO_PORTD_AMSEL_R = 0;								// Disable analog functionality on Port D
	SYSCTL_RCGCSSI_R |= 0x02;							// Activate SSI1
	SYSCTL_RCGCGPIO_R |= 0x08;						// Activate port D
	while((SYSCTL_PRGPIO_R&0x08) == 0){};	// Wait until ready.
	SSI1_Init();
	SSI1_CR1_R = 0x00000000;							// Disable SSI, master mode
	SSI1_CPSR_R = 0x02;										// fSSI = fBUS/(CPSDVSR  ( 1 + SCR) = fSSI = 50MHz/4 = 12MHz SSIClk
	SSI1_CR0_R &= ~(0x0000FFF0);					// SCR = 0, SPH = 0, Freescale
	SSI1_CR0_R |= SSI_CR0_SPO;						// SPO = 1 (flip clock bit)
	SSI1_CR0_R |= 0x0B;										// DSS = 12-bit data
}

// ***************** SSI2_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void SSI2_Init(){
	GPIO_PORTB_DIR_R |= 0x20;   // make PD1 out (FS output)
  GPIO_PORTB_AFSEL_R &= ~0x40;// disable alt funct on PF2
	GPIO_PORTB_AFSEL_R |= 0x90;						// Enable alt. function on PD0, 3
	GPIO_PORTB_DEN_R |= 0xB0;							// Configure PD0, 1, 3 as SSI
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFF0F00F) + 0x00020020; // Write 2 to port to signify SSI.
	GPIO_PORTB_AMSEL_R = 0;								// Disable analog functionality on Port D
	SYSCTL_RCGCSSI_R |= 0x04;							// Activate SSI1
	SYSCTL_RCGCGPIO_R |= 0x02;						// Activate port B
	while((SYSCTL_PRGPIO_R&0x02) == 0){};	// Wait until ready.
	SSI2_Init();
	SSI2_CR1_R = 0x00000000;							// Disable SSI, master mode
	SSI2_CPSR_R = 0x02;										// fSSI = fBUS/(CPSDVSR  ( 1 + SCR) = fSSI = 50MHz/4 = 12MHz SSIClk
	SSI2_CR0_R &= ~(0x0000FFF0);					// SCR = 0, SPH = 0, Freescale
	SSI2_CR0_R |= SSI_CR0_SPO;						// SPO = 1 (flip clock bit)
	SSI2_CR0_R |= 0x0B;										// DSS = 12-bit data
}


// ***************** DAC_Init ****************
// Initializes DAC output pin and all timers used to produce sound.
// Inputs:  none
// Outputs: none
void DAC_Init(uint16_t data){
  // Maybe disable interrupts here
	SSI1_Init();
	SSI2_Init();
	GPIO_PORTD_DATA_R |= 0x02;						// Toggle FS on to start latching data into DAC.
	SSI1_DR_R = data & 0x0FFF;						// Load data into TX FIFO
	while(~((SSI1_CR1_R) & 0x00000010)){}; // SSI SSI1 TX 12-bit data TX not complete
	SSI1_DR_R = (data >> 8) & 0x000F;
	while(~((SSI1_CR1_R) & 0x00000010)){}; // SSI1 TX not complete
	GPIO_PORTD_DATA_R &= ~0x0002;						// Toggle SSI1 FS off to stop latching data.
	GPIO_PORTB_DATA_R |= 0x20;						// Toggle SSI2 FS on to start latching data into DAC.
	SSI2_DR_R = data & 0x0FFF;						// Load data into TX FIFO
	while(~((SSI2_CR1_R) & 0x00000010)){}; // SSI2 TX 12-bit data TX not complete
	SSI2_DR_R = (data >> 8) & 0x000F;
	while(~((SSI2_CR1_R) & 0x00000010)){}; // SSI2 TX not complete
	GPIO_PORTB_DATA_R &= ~0x0020;						// Toggle SSI2 FS off to stop latching data.

	SSI1_CR1_R |= 0x00000002;							// Enable SSI1
	SSI2_CR1_R |= 0x00000002;							// Enable SSI2
}

// ***************** DAC1_Out ****************
// Outputs a voltage value to the DAC
// Inputs:  none
// Outputs: none
void DAC1_Out(uint16_t code){
	// Maybe disable interrupts here?
  GPIO_PORTD_DATA_R |= 0x02;						// Toggle FS on to start latching data into DAC.
	SSI1_DR_R = code & 0x0FFF;
	while(~((SSI1_CR1_R) & 0x00000010)){}; // SSI TX 12-bit data TX not complete
	SSI1_DR_R = (code >> 8) & 0x000F;
	while(~((SSI1_CR1_R) & 0x00000010)){}; // SSI TX 12-bit data TX not complete
	//while((SSI1_SR_R & 0x00000002) == 0){}; // SSI TX FIFO not full.
	GPIO_PORTD_DATA_R &= ~0x0002;		
	// Toggle FS off to stop latching data.
	//SSI1_DR_R = code;
}

// ***************** DAC2_Out ****************
// Outputs a voltage value to the DAC
// Inputs:  none
// Outputs: none
void DAC2_Out(uint16_t code){
	// Maybe disable interrupts here?
  GPIO_PORTB_DATA_R |= 0x20;						// Toggle FS on to start latching data into DAC.
	SSI2_DR_R = code & 0x0FFF;
	while(~((SSI2_CR1_R) & 0x00000010)){}; // SSI TX 12-bit data TX not complete
	SSI2_DR_R = (code >> 8) & 0x000F;
	while(~((SSI2_CR1_R) & 0x00000010)){}; // SSI TX 12-bit data TX not complete
	//while((SSI2_SR_R & 0x00000002) == 0){}; // SSI TX FIFO not full.
	GPIO_PORTB_DATA_R &= ~0x20;		
	// Toggle FS off to stop latching data.
	//SSI1_DR_R = code;
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
// Debugging test to allow stepping through waveform.
// Inputs:  none
// Outputs: none
void Simple_DAC_Test()
{
	uint32_t sampleNotes[8] = {0,64,128,256,512,1024,1536,2048};
	for(int i =0; i < 8; i++)
	{
		DAC1_Out(sampleNotes[i]);
		DAC2_Out(sampleNotes[(i + 1) % 8];
		DelayWait5ms(1);
	}
}
