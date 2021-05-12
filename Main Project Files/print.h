// Print.h
// This software has interface for printing
// Runs on TM4C123
// Program written by: put your names here
// Date Created: 
// Last Modified:  
// Lab number: 7

#ifndef PRINT_H
#define PRINT_H
#include <stdint.h>

//-----------------------LCD_OutDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: n is the number to output
// Output: none
char *LCD_OutDec(uint32_t n);
void LCD_OutDecCoin(uint32_t coin);

//-----------------------LCD_OutFix-----------------------
// Output a 32-bit number in unsigned fixed-point format
// Input: m is the integer part of the fixed point number to output
// Output: none
// m=0,    then output "0.00 "
// m=3,    then output "0.03 "
// m=89,   then output "0.89 "
// m=123,  then output "1.23 "
// m=999,  then output "9.99 "
// m>999,  then output "*.** "
void LCD_OutFix(uint32_t m);

#endif
