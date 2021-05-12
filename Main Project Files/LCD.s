; LCD.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly

; Runs on TM4C123
; Use I2C3 to send data to SSD1306 128 by 64 pixel oLED

; As part of Lab 7, students need to implement I2C_Send2

      EXPORT   I2C_Send2
      PRESERVE8
      AREA    |.text|, CODE, READONLY, ALIGN=2
      THUMB
      ALIGN
I2C3_MSA_R  EQU 0x40023000
I2C3_MCS_R  EQU 0x40023004
I2C3_MDR_R  EQU 0x40023008
; sends two bytes to specified slave
; Input: R0  7-bit slave address
;        R1  first 8-bit data to be written.
;        R2  second 8-bit data to be written.
; Output: 0 if successful, nonzero (error code) if error
; Assumes: I2C3 and port D have already been initialized and enabled
I2C_Send2
;; --UUU-- 
; 1) wait while I2C is busy, wait for I2C3_MCS_R bit 0 to be 0
busy
	LDR R3,= I2C3_MCS_R ;Load R3 with address of MCS register
	LDR R3, [R3] ;Load R3 the data in MCS regsiter
	AND R3, #0x01 ;Extract bit 0 out of the MCS register
	CMP R3, #0 ;Compare R3 with 0 and update CC
	BNE busy ;Check if R3 is not equal to 0
; 2) write slave address to I2C3_MSA_R, 
;     MSA bits7-1 is slave address
;     MSA bit 0 is 0 for send data
loop
	LSL R0, #1 ;Left shift R0 by 1 bit to get slave address in bits 1-7
	AND R12, R0, #0x01 ;Extract out bit 0
	CMP R12, #0 ;Compare R12 with 0 and update CC
	BNE loop ;Check if R12 is not 0
	LDR R3,= I2C3_MSA_R ;Load R3 with the address of MSA register
	STR R0, [R3] ;Store R0 into MSA register
; 3) write first data to I2C3_MDR_R
	LDR R3,= I2C3_MDR_R ;Load R3 with MDR register
	STR R1, [R3] ;Store first data into the MDR register
; 4) write 0x03 to I2C3_MCS_R,  send no stop, generate start, enable
	LDR R3,= I2C3_MCS_R ;Load R3 with the address of the MCS register
	MOV R12, #0x03 ;Load R12 with 0x03
	STR R12, [R3] ;Store 0x03 into the MCS register
; add 4 NOPs to wait for I2C to start transmitting
	NOP
	NOP
	NOP
	NOP
; 5) wait while I2C is busy, wait for I2C3_MCS_R bit 0 to be 0
busy1
	LDR R3,= I2C3_MCS_R ;Load R3 with the address of the MCS register
	LDR R3, [R3] ;Load R3 with the data in the MCS register
	AND R3, #0x01 ;Extract bit 0 of the MCS register
	CMP R3, #0 ;Compare R3 with 0 and update CC
	BNE busy1 ;Check if R3 is not equal to 0
; 6) check for errors, if any bits 3,2,1 I2C3_MCS_R is high 
;    a) if error set I2C3_MCS_R to 0x04 to send stop 
;    b) if error return R0 equal to bits 3,2,1 of I2C3_MCS_R, error bits
	LDR R3,= I2C3_MCS_R 
	LDR R3, [R3]
	ASR R3, #1
	AND R3, #0x07
	CMP R3, #0
	BEQ next

	LDR R0, [R3]
	MOV R12, #0x04
	LDR R3,= I2C3_MCS_R
	STR R12, [R3]
	BX LR
next
; 7) write second data to I2C3_MDR_R
	LDR R3,= I2C3_MDR_R
	STR R2, [R3]
; 8) write 0x05 to I2C3_MCS_R, send stop, no start, enable
	LDR R3,= I2C3_MCS_R
	MOV R12, #0x05
	STR R12, [R3]
; add 4 NOPs to wait for I2C to start transmitting
	NOP
	NOP
	NOP
	NOP
; 9) wait while I2C is busy, wait for I2C3_MCS_R bit 0 to be 0
busy2
	LDR R3,= I2C3_MCS_R
	LDR R3, [R3]
	AND R3, #0x01
	CMP R3, #0
	BNE busy2
; 10) return R0 equal to bits 3,2,1 of I2C3_MCS_R, error bits
;     will be 0 is no error
	LDR R3,= I2C3_MCS_R
	LDR R3, [R3]
	ASR R3, #1
	AND R3, #0x07
	LDR R0, [R3]

    BX  LR                          ;   return


    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
 