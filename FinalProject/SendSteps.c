

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/tm4c123gh6pm.h"

#include "pll.h"
#include "UART.h"
#include "esp8266.h"
#include "PLL.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PWMSine.h"
#include "Switch.h"
#include "FIFOqueue.h"
#include "Timer0A.h"
#include "Timer1.h"
#include "Timer3.h"
#include "Music.h"
#include "DAC.h"
#include "Heap.h"
#include "Accelerometer.h"
#include "AccelAvgFIFO.h"
#include "SendSteps.h"


#define SENDSTRING1 "GET /query?city=Austin%2C%20Texas&id=Kevin%20and%20Stefan&greet=" 
#define SENDSTRING2 " HTTP/1.1\r\nUser-Agent: Keil\r\nHost: ee445l-kjy252.appspot.com\r\n\r\n"
char Fetch[] = "GET /data/2.5/weather?q=Austin%20Texas&APPID=1bc54f645c5f1c75e681c102ed4bbca4&units=metric HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n";

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID
/*Helper function to turn ADC Data into string format */

void itoa(uint32_t voltage, char buffer[]){
	char const digits[] = "0123456789";
	char* pos = buffer;
	int counter = voltage;
	do{
		++pos;
		counter = counter/10;
	}while(counter);
	*pos = '\0';
	do{
		*--pos = digits[voltage%10];
		voltage /= 10;
}while(voltage);
}

void sendSteps(uint32_t Steps)
{
	char voltagebuffer[10];
	char TCPPACKET[200] ="";
	itoa(Steps,voltagebuffer);
	
	strcat(TCPPACKET,SENDSTRING1);
	strcat(TCPPACKET,voltagebuffer);
	strcat(TCPPACKET,SENDSTRING2);
	
	ESP8266_SendTCP(TCPPACKET);

}
