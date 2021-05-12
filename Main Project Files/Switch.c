// Switch.c
// This software to input from switches or buttons
// Runs on TM4C123
// Program written by: put your names here
// Date Created: 3/6/17 
// Last Modified: 1/14/21
// Lab number: 10
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "Sound.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
extern uint32_t ScoreTracker;
volatile uint8_t RisingEdges = 0;
volatile uint8_t FallingEdges = 0;
volatile uint8_t TogglerSelection = 0;
volatile uint8_t PausePlay;
volatile uint8_t NextButton = 0;
uint32_t ScoreStorage;
void EdgeCounter_Init(void){

	SYSCTL_RCGCGPIO_R |= 0x00000010; //Activate clock for port E
	GPIO_PORTE_DIR_R &= ~0x0F; //Make PE0-3 Input
	GPIO_PORTE_AFSEL_R &= ~0x0F; //Disable alt function on PE0-3
	GPIO_PORTE_DEN_R |= 0x0F; //Enable digital I/O pn PE0-3
	GPIO_PORTE_PCTL_R &= ~0x0000FFFF; //Configure PE0-3 as GPIO
	GPIO_PORTE_AMSEL_R &= ~0x0F; //Disable analog functionality on PE0-3
	GPIO_PORTE_IS_R &= ~0x0F; //PE0-3 is edge sensitive
	GPIO_PORTE_IBE_R |= 0x01; //PE0 is both edges interrupt
	GPIO_PORTE_IEV_R |= 0x06; //PE1-2 is a rising edge interrupt
	GPIO_PORTE_IEV_R &= ~0x10; //PE3 is a falling edge interrupt
	GPIO_PORTE_ICR_R = 0x0F; //Clear Flag 0-3
	GPIO_PORTE_IM_R |= 0x0F; //Arm interrupt on PE0-3
	NVIC_PRI1_R = (NVIC_PRI1_R&0xFF00FFFF)|0x00A00000; //Priority 5
	NVIC_EN0_R = 0x00000010; //Enable interrupt 30 in NVIC
}

void PausePlayButton(void){
	if(TogglerSelection == 1){
		PausePlay = 1;
	}
	else{
		PausePlay = 0;
	}
}

void GPIOPortE_Handler(void){
	GPIO_PORTE_ICR_R = 0x0F; //Acknowledge Flag0-3
	if((GPIO_PORTE_DATA_R&0x01) == 0x01){
		RisingEdges = 1;
		FallingEdges = 0;
	}
	else if((GPIO_PORTE_DATA_R&0x01) == 0x00){
		FallingEdges = 1;
		RisingEdges = 0;
	}
	
	if((GPIO_PORTE_DATA_R&0x02) == 0x02){
		ScoreStorage = ScoreTracker;
		TogglerSelection ^= 1;
		PausePlayButton();
	}
	
	if((GPIO_PORTE_DATA_R&0x04) == 0x04){
		NextButton++;
		if(NextButton == 2){
			NextButton = 0;
		}
	}
}
