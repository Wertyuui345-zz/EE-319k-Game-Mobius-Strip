// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 1/14/21 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 8-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	SYSCTL_RCGC2_R |= 0x02; //Port B on
	SYSCTL_RCGCGPIO_R |= 0x02;
	__nop(); 
	__nop();
	GPIO_PORTB_DIR_R |= 0xFF; //Port B Output
	GPIO_PORTB_DEN_R |= 0xFF; //Port B Digital
}

// **************DAC_Out*********************
// output to DAC
// Input: 8-bit data, 0 to 255
// Input=n is converted to n*3.3V/255
// Output: none
void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R = data;
}
