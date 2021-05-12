; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
	EXPORT	 LCD_OutDecCoin
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB


;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
Pointer RN 12
MAXDIV EQU 1000000000
LCD_OutDec
	SUB SP, #12; 12 1 byte containers for each digit
	MOV Pointer, SP; Setting R11 as pointer, FP 
	PUSH{R4-R6,LR}

	MOV R2, #0; CounterBit
	LDR R4, =MAXDIV
	MOV R5, #10
	MOV R1, R0
DecimalLoop
	MOV R3, R1; R1 contains total sum 
	UDIV R3, R4; Sum divided by R4
	STRB R3, [Pointer, R2]; That sum is stored in pointer digit 
	MUL R3, R4; Divisor multiplied by total number of that place
	SUB R1, R3; R1 now holds the sum minus the digits accounted for
	ADD R2, #1; Counter increment
	UDIV R4, R5; Move to next digit
	CMP R2, #10; Jump if max digits
	BNE DecimalLoop
	
	MOV R1, #0
	STRB R1, [Pointer, #10]; Sentinel bit
;Where is the top of the stack
FindTop
	LDRB R3, [Pointer, R1]
	ADD R1, #1
	CMP R1, #10
	BEQ FirstDigit
	CMP R3, #0
	BEQ FindTop; Pointer + R1 - 1 points to Stack top

FirstDigit
	SUB R1, #1; Fixing counter to be accurate
	MOV R0, #0
	ADD R0, Pointer, R1; Top of the stack in R0, for BL Out
	
ASCIIReplace
	LDRB R3, [Pointer, R1]; Convert Everything to Ascii
	ADD R3, #0x30
	STRB R3, [Pointer, R1]
	ADD R1, #1
	CMP R1, #10
	BNE ASCIIReplace
	POP{R4-R6,LR}
	ADD SP, #12
	MOV R0, R0
	BX LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

N EQU 0
CNT EQU 4
LCD_OutDecCoin
	SUB SP, #8
	MOV R11, SP
	PUSH{LR, R4}
	STR R0, [R11, #N]
	MOV R1, #0
	STR R1, [R11, #CNT]
	MOV R1, #10
loop	
	LDR R2, [R11, #CNT]
	ADD R2, R2, #1
	STR R2, [R11, #CNT]
	
	LDR R3, [R11, #N]
	MOV R4, R3
	UDIV R3, R3, R1
	STR R3, [R11, #N]
	MUL R3, R3, R1
	SUB R4, R4, R3
	PUSH{R4}
	LDR R3, [R11, #N]
	CMP R3, #0
	BNE loop
BACK	
	POP{R4}
	ADD R0, R4, #0x30
	BL SSD1306_OutChar
	LDR R2, [R11, #CNT]
	SUB R2, R2, #1
	STR R2, [R11, #CNT]
	CMP R2, #0
	BNE BACK
	
	POP{LR, R4}
	ADD SP, #8
	
      BX  LR

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
