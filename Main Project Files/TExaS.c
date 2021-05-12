// TExaS.c
// Runs on TM4C123
// Periodic timer interrupt data collection
// PLL turned on at 80 MHz
// Implements Logic Analyzer or scope
// Timer5A periodic interrupt implements logic analyzer
// Uses ADC1 and PD2

// Jonathan Valvano, Daniel Valvano
// January 16, 2021

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void PLL_Init(void);
void UART0_Init(void);
void ADC1_Init(void);

void (*sendDataPt)(void);


// ************TExaS_Init*****************
// Initialize 7-bit logic analyzer on timer 5A 100us
// sets PLL to 80 MHz
// This needs to be called once
// Inputs: function to send data
// This will only activate clock, user sets direction and other modes
// Outputs: none
void TExaS_Init(void(*task)(void)){
  PLL_Init();     // PLL on at 80 MHz
  ADC1_Init();
  sendDataPt = task;

  SYSCTL_RCGCTIMER_R |= 0x20;      // 0) activate timer5
  UART0_Init();                    // UART0 is connected to TExaSdisplay
  TIMER5_CTL_R = 0x00000000;       // 1) disable timer5A during setup
  TIMER5_CFG_R = 0x00000000;       // 2) configure for 32-bit mode
  TIMER5_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
  TIMER5_TAILR_R = 7999;           // 4) 100us reload value
  TIMER5_TAPR_R = 0;               // 5) bus clock resolution
  TIMER5_ICR_R = 0x00000001;       // 6) clear timer5A timeout flag
  TIMER5_IMR_R = 0x00000001;       // 7) arm timeout interrupt
  NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|0x00000040; // 8) priority 2
// interrupts enabled in the main program after all devices initialized
// vector number 108, interrupt number 92
  NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 92 in NVIC
  TIMER5_CTL_R = 0x00000001;       // 10) enable timer5A
}

// Timer5 implements the logic analyzer
// Sends 7-bit data to PC running TExaSdisplay via the USB cable
void Timer5A_Handler(void){
  TIMER5_ICR_R = 0x00000001;         // acknowledge timer5A timeout
  (*sendDataPt)();
}


// ************TExaS_Stop*****************
// Stop the transfer
// Inputs:  none
// Outputs: none
void TExaS_Stop(void){
  NVIC_DIS2_R = 0x10000000;       // 9) disable interrupt 92 in NVIC
  TIMER5_CTL_R = 0x00000000;      // 10) disable timer5A
}


// The #define statement SYSDIV2 specifies
// the PLL to the desired frequency.

// bus frequency is 400MHz/(SYSDIV2+1) = 400MHz/(4+1) = 80 MHz
// see the table at the end of this file

#define SYSCTL_RIS_PLLLRIS      0x00000040  // PLL Lock Raw Interrupt Status
#define SYSCTL_RCC_XTAL_M       0x000007C0  // Crystal Value
#define SYSCTL_RCC_XTAL_6MHZ    0x000002C0  // 6 MHz Crystal
#define SYSCTL_RCC_XTAL_8MHZ    0x00000380  // 8 MHz Crystal
#define SYSCTL_RCC_XTAL_16MHZ   0x00000540  // 16 MHz Crystal
#define SYSCTL_RCC2_USERCC2     0x80000000  // Use RCC2
#define SYSCTL_RCC2_DIV400      0x40000000  // Divide PLL as 400 MHz vs. 200
                                            // MHz
#define SYSCTL_RCC2_SYSDIV2_M   0x1F800000  // System Clock Divisor 2
#define SYSCTL_RCC2_SYSDIV2LSB  0x00400000  // Additional LSB for SYSDIV2
#define SYSCTL_RCC2_PWRDN2      0x00002000  // Power-Down PLL 2
#define SYSCTL_RCC2_BYPASS2     0x00000800  // PLL Bypass 2
#define SYSCTL_RCC2_OSCSRC2_M   0x00000070  // Oscillator Source 2
#define SYSCTL_RCC2_OSCSRC2_MO  0x00000000  // MOSC
// The #define statement SYSDIV2 initializes
// the PLL to the desired frequency.
#define SYSDIV2 4
// bus frequency is 400MHz/(SYSDIV2+1) = 400MHz/(4+1) = 80 MHz

// configure the system to get its clock from the PLL
void PLL_Init(void){
  // 0) configure the system to use RCC2 for advanced features
  //    such as 400 MHz PLL and non-integer System Clock Divisor
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   // clear XTAL field
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;// configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;// clear oscillator source field
  SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;// configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
  // 4) set the desired system divider and the system divider least significant bit
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;  // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) // clear system clock divider field
                  + (SYSDIV2<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){};
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable
#define UART_CC_CS_M            0x0000000F  // UART Baud Clock Source
#define UART_CC_CS_SYSCLK       0x00000000  // The system clock (default)
#define SYSCTL_RCGC1_UART0      0x00000001  // UART0 Clock Gating Control
#define SYSCTL_RCGC2_GPIOA      0x00000001  // port A Clock Gating Control
#define SYSCTL_PRGPIO_R0        0x00000001  // GPIO Port A Peripheral Ready
#define SYSCTL_PRUART_R0        0x00000001  // UART Module 0 Peripheral Ready
//------------UART0_Init------------
// Initialize the UART for 115,200 baud rate (assuming 80 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART0_Init(void){volatile unsigned long delay;
  SYSCTL_RCGCUART_R |= 0x01; // activate UART0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGC2_GPIOA; // activate port A
                                        // wait for clock to stabilize
//  while((SYSCTL_PRUART_R&SYSCTL_PRUART_R0) == 0){};
  delay = SYSCTL_RCGCGPIO_R;
  delay = SYSCTL_RCGCGPIO_R;
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 43;                    // IBRD = int(80,000,000 / (16 * 115200)) = int(43.402778)
  UART0_FBRD_R = 26;                    // FBRD = round(0.402778 * 64) = 26
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CC_R &= ~UART_CC_CS_M;          // clear baud clock control field
  UART0_CC_R |= UART_CC_CS_SYSCLK;      // configure for system clock
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
//  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R0) == 0){};
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
                                        // configure PA1-0 as UART
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}


// start conversions, sample always
// ADC1
// PD3 Ain4
// 16-point averaging 125kHz sampling
void ADC1_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGCADC_R |= 0x02;       // 1) ADC1 clock
  SYSCTL_RCGCGPIO_R |= 0x08;      // 2) activate clock for Port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// allow time for clock to stabilize
  GPIO_PORTD_DIR_R &= ~0x08;      // 3) make PD3 input
  GPIO_PORTD_AFSEL_R |= 0x08;     // 4) enable alternate function on PD3
  GPIO_PORTD_DEN_R &= ~0x08;      // 5) disable digital I/O on PD3
  GPIO_PORTD_AMSEL_R |= 0x08;     // 6) enable analog functionality on PD3
  for(delay = 0; delay<20; delay++){};  // allow time for clock to stabilize
  ADC1_PC_R = 0x01;               // 7) 125K rate
  ADC1_SSPRI_R = 0x0123;          // 8) Sequencer 3 is highest priority
  ADC1_ACTSS_R = 0x0000;          // 9) disable sample sequencer 3
  ADC1_EMUX_R |= 0xF000;          // 10) seq3 is always/continuous trigger
  ADC1_SAC_R = 0x03;              //   8-point average 125kHz/8 = 15,625 Hz
  ADC1_SSMUX3_R = 4;              // 11) set channel 4
  ADC1_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
  ADC1_IM_R = 0x0000;             // 13) disable SS3 interrupts
  ADC1_ACTSS_R = 0x0008;          // 14) enable sample sequencer 3
}


