#include "m16def.inc"

.DSEG
_tmp_:.byte 2
.CSEG

.org 0x0
rjmp reset

reset:
	ldi r24 ,(1 << PC7) | (1 << PC6) | (1 << PC5) | (1 << PC4) 
	out DDRC ,r24 
	ser r24
	out DDRB, r24
	ldi	r16,LOW(RAMEND)		; load low byte of RAMEND into r16
	out	SPL,r16			; store r16 in stack pointer low
	ldi	r16,HIGH(RAMEND)	; load high byte of RAMEND into r16
	out	SPH,r16		
	ser r24
	out DDRD, r24 


main:
call read_temp      ;Reading Temperature
cpi r25,0x80
brne device							
cpi r24,0x00        ;If no device found 
brne device			;aka 8000 was returned
call no_de			;print "no Device"
rjmp main			
device:				
call display		;else display the Temperature
rjmp main

no_de:				;printing "NO Device"
rcall lcd_init						
ldi r24,'N'												
rcall lcd_data	
ldi r24,'O'												
rcall lcd_data	
ldi r24,' '												
rcall lcd_data	
ldi r24,'D'												
rcall lcd_data	
ldi r24,'e'												
rcall lcd_data	
ldi r24,'v'												
rcall lcd_data	
ldi r24,'i'												
rcall lcd_data	
ldi r24,'c'												
rcall lcd_data	
ldi r24,'e'												
rcall lcd_data	
rjmp main


display:
mov r21,r24							
ldi r18,'+'							;initialize r18 with '+' (r18 sign register)
sbrs r21,7
rjmp pos
ldi r18,'-'							;if the result number is negative make r18 = '-'
dec r21
com r21
pos:
lsr r21
ldi r20, 0x00						;(r20 hundreds register)
clr r19
nextp:
cpi r21,0x0A						;(r19 doxens register)
brlo fin							;Checking for the doxens
inc r19
subi r21,0x0A
rjmp nextp
fin:
rcall lcd_init						;starting printing on the LCD display the
ldi r22,0x30						;result in the format
mov r24,r18							;r18  r20   r19   r21
rcall lcd_data						;each of them containing the char
add r20,r22	
mov r24,r20
rcall lcd_data 
add r19,r22
mov r24,r19
rcall lcd_data 
add r21,r22
mov r24,r21
rcall lcd_data 
ldi r24,0xB0
rcall lcd_data 
ldi r24,'C'
rcall lcd_data 
ret






read_temp:
call one_wire_reset
cpi r24,0x00
brne device_de
ldi r25, 0x80
ldi r24,0x00
ret
device_de:
ldi r24,0xCC
call one_wire_transmit_byte
ldi r24,0x44
call one_wire_transmit_byte
wait:
call one_wire_receive_bit
sbrs r24,0
rjmp wait
call one_wire_reset
ldi r24,0xCC
call one_wire_transmit_byte
ldi r24,0xBE
call one_wire_transmit_byte

call one_wire_receive_byte
mov r20,r24
call one_wire_receive_byte
mov r25,r24
mov r24,r20
ret

















wait_msec:			
 	push r24 
 	push r25 
 	ldi r24 , low(998) 	
 	ldi r25 , high(998) 	
 	rcall wait_usec 
 	pop r25 
 	pop r24 
 	sbiw r24 , 1 
 	brne wait_msec 
 	ret 

wait_usec:
	sbiw r24 ,1 
	nop 
	nop 
	nop 

	nop
	brne wait_usec 
 	ret
end:



; File Name: one_wire.asm
; Title: one wire protocol
; Target mcu: atmega16
; Development board: easyAVR6
; Assembler: AVRStudio assembler
; Description:
; This file includes routines implementing the one wire protocol over the PA4 pin of the microcontroller.
; Dependencies: wait.asm

; Routine: one_wire_receive_byte
; Description

; This routine generates the necessary read
; time slots to receives a byte from the wire.
; return value: the received byte is returned in r24.
; registers affected: r27:r26 ,r25:r24
; routines called: one_wire_receive_bit
one_wire_receive_byte:
ldi r27 ,8
clr r26
loop_:
rcall one_wire_receive_bit
lsr r26
sbrc r24 ,0
ldi r24 ,0x80
or r26 ,r24
dec r27
brne loop_
mov r24 ,r26
ret

; Routine: one_wire_receive_bit
; Description

; This routine generates a read time slot across the wire.
; return value: The bit read is stored in the lsb of r24.
; if 0 is read or 1 if 1 is read.
; registers affected: r25:r24
; routines called: wait_usec
one_wire_receive_bit:

sbi DDRA ,PA4
cbi PORTA ,PA4 ; generate time slot
ldi r24 ,0x02
ldi r25 ,0x00
rcall wait_usec
cbi DDRA ,PA4 ; release the line
cbi PORTA ,PA4
ldi r24 ,10 ; wait 10 �s
ldi r25 ,0
rcall wait_usec
clr r24 ; sample the line
sbic PINA ,PA4
ldi r24 ,1
push r24
ldi r24 ,49 ; delay 49 �s to meet the standards
ldi r25 ,0 ; for a minimum of 60 �sec time slot
rcall wait_usec ; and a minimum of 1 �sec recovery time
pop r24
ret

; Routine: one_wire_transmit_byte
; Description:
; This routine transmits a byte across the wire.
; parameters:
; r24: the byte to be transmitted must be stored here.
; return value: None.
; registers affected: r27:r26 ,r25:r24
; routines called: one_wire_transmit_bit
one_wire_transmit_byte:
mov r26 ,r24
ldi r27 ,8
_one_more_:
clr r24
sbrc r26 ,0
ldi r24 ,0x01
rcall one_wire_transmit_bit
lsr r26
dec r27
brne _one_more_
ret

; Routine: one_wire_transmit_bit
; Description:
; This routine transmits a bit across the wire.
; parameters:
; r24: if we want to transmit 1
; then r24 should be 1, else r24 should
; be cleared to transmit 0.
; return value: None.
; registers affected: r25:r24
; routines called: wait_usec
one_wire_transmit_bit:
push r24 ; save r24
sbi DDRA ,PA4
cbi PORTA ,PA4 ; generate time slot
ldi r24 ,0x02
ldi r25 ,0x00
rcall wait_usec
pop r24 ; output bit
sbrc r24 ,0
sbi PORTA ,PA4
sbrs r24 ,0
cbi PORTA ,PA4
ldi r24 ,58 ; wait 58 �sec for the
ldi r25 ,0 ; device to sample the line
rcall wait_usec
cbi DDRA ,PA4 ; recovery time
cbi PORTA ,PA4
ldi r24 ,0x01

ldi r25 ,0x00
rcall wait_usec
ret
; Routine: one_wire_reset
; Description

; This routine transmits a reset pulse across the wire
; and detects any connected devices.
; parameters: None.
; return value: 1 is stored in r24
; if a device is detected, or 0 else.
; registers affected r25:r24
; routines called: wait_usec
one_wire_reset:
sbi DDRA ,PA4 ; PA4 configured for output
cbi PORTA ,PA4 ; 480 �sec reset pulse
ldi r24 ,low(480)
ldi r25 ,high(480)
rcall wait_usec
cbi DDRA ,PA4 ; PA4 configured for input
cbi PORTA ,PA4
ldi r24 ,100 ; wait 100 �sec for devices
ldi r25 ,0 ; to transmit the presence pulse
rcall wait_usec
in r24 ,PINA ; sample the line
push r24
ldi r24 ,low(380) ; wait for 380 �sec
ldi r25 ,high(380)
rcall wait_usec
pop r25 ; return 0 if no device was
clr r24 ; detected or 1 else
sbrs r25 ,PA4
ldi r24 ,0x01
ret



scan_row:
ldi r25 , 0x08
back_:
lsl r25			
dec r24			
brne back_
out PORTC, r25 
nop
nop				
in r24 , PINC	
andi r24 ,0x0F	
ret




scan_keypad:
ldi r24 , 0x01 
rcall scan_row
swap r24 
mov r27 , r24
ldi r24 ,0x02
rcall scan_row
add r27 , r24 

ldi r24 , 0x03 
rcall scan_row
swap r24
mov r26 , r24

ldi r24 ,0x04 
rcall scan_row
add r26 , r24 
movw r24 , r26 
ret
















keypad_to_ascii: ; ?????? �1� st?? ??se?? t?? ?ata????t? r26 d???????
movw r26 ,r24 ; ta pa?a??t? s?��??a ?a? a???�???
ldi r24 ,'E'
sbrc r26 ,0
ret
ldi r24 ,'0'
sbrc r26 ,1
ret
ldi r24 ,'F'
sbrc r26 ,2
ret
ldi r24 ,'D'
sbrc r26 ,3 ; a? de? e??a? �1�pa?a??�pte? t?? ret, a????? (a? e??a? �1�)
ret ; ep?st??fe? �e t?? ?ata????t? r24 t?? ASCII t?�? t?? D.
ldi r24 ,'7'
sbrc r26 ,4
ret
ldi r24 ,'8'
sbrc r26 ,5
ret
ldi r24 ,'9'
sbrc r26 ,6
ret
ldi r24 ,'C'
sbrc r26 ,7
ret

ldi r24 ,'4' ; ?????? �1� st?? ??se?? t?? ?ata????t? r27 d???????
sbrc r27 ,0 ; ta pa?a??t? s?��??a ?a? a???�???
ret
ldi r24 ,'5'
sbrc r27 ,1
ret
ldi r24 ,'6'
sbrc r27 ,2
ret
ldi r24 ,'B'
sbrc r27 ,3
ret
ldi r24 ,'1'
sbrc r27 ,4
ret
ldi r24 ,'2'
sbrc r27 ,5
ret
ldi r24 ,'3'
sbrc r27 ,6
ret
ldi r24 ,'A'
sbrc r27 ,7
ret
clr r24
ret



write_2_nibbles:
push r24 ; st???e? ta 4 MSB
in r25 ,PIND ; d?a�????ta? ta 4 LSB ?a? ta ?a?ast?????�e
andi r25 ,0x0f ; ??a ?a �?? ?a??s??�e t?? ?p??a p??????�e?? ?at?stas?
andi r24 ,0xf0 ; ap?�??????ta? ta 4 MSB ?a?
add r24 ,r25 ; s??d?????ta? �e ta p???p?????ta 4 LSB
out PORTD ,r24 ; ?a? d????ta? st?? ???d?
sbi PORTD ,PD3 ; d?�?????e?ta? pa?�?? Enable st?? a???d??t? PD3
cbi PORTD ,PD3 ; PD3=1 ?a? �et? PD3=0
pop r24 ; st???e? ta 4 LSB. ??a?t?ta? t? byte.
swap r24 ; e?a???ss??ta? ta 4 MSB �e ta 4 LSB
andi r24 ,0xf0 ; p?? �e t?? se??? t??? ap?st?????ta?
add r24 ,r25
out PORTD ,r24
sbi PORTD ,PD3 ; ???? pa?�?? Enable
cbi PORTD ,PD3
ret
lcd_command:
cbi PORTD ,PD2 ; ep????? t?? ?ata????t? e?t???? (PD2=1)
rcall write_2_nibbles ; ap?st??? t?? e?t???? ?a? a?a�??? 39�sec
ldi r24 ,39 ; ??a t?? ????????s? t?? e?t??es?? t?? ap? t?? e?e??t? t?? lcd.
ldi r25 ,0 ; S??.: ?p?????? d?? e?t????, ?? clear display ?a? return home,
rcall wait_usec ; p?? apa?t??? s?�a?t??? �e?a??te?? ??????? d??st?�a.
ret
lcd_data:
sbi PORTD ,PD2 ; ep????? t?? ?ata????t? ded?�???? (PD2=1)
rcall write_2_nibbles ; ap?st??? t?? byte
ldi r24 ,43 ; a?a�??? 43�sec �???? ?a ?????????e? ? ????
ldi r25 ,0 ; t?? ded?�???? ap? t?? e?e??t? t?? lcd
rcall wait_usec
ret
lcd_init:
ldi r25,0
ldi r24 ,40 ; ?ta? ? e?e??t?? t?? lcd t??f?d?te?ta? �eldi r25 ,0 ; ?e?�a e?te?e? t?? d??? t?? a?????p???s?.
rcall wait_msec ; ??a�??? 40 msec �???? a?t? ?a ?????????e?.
ldi r24 ,0x30 ; e?t??? �et?�as?? se 8 bit mode
out PORTD ,r24 ; epe?d? de? �p????�e ?a e?�aste �?�a???
sbi PORTD ,PD3 ; ??a t? d?a�??f?s? e?s?d?? t?? e?e??t?
cbi PORTD ,PD3 ; t?? ??????, ? e?t??? ap?st???eta? d?? f????
ldi r24 ,39
ldi r25 ,0 ; e?? ? e?e??t?? t?? ?????? �??s?eta? se 8-bit mode
rcall wait_usec ; de? ?a s?��e? t?p?ta, a??? a? ? e?e??t?? ??e? d?a�??f?s?
; e?s?d?? 4 bit ?a �eta�e? se d?a�??f?s? 8 bit
ldi r24 ,0x30
out PORTD ,r24
sbi PORTD ,PD3
cbi PORTD ,PD3
ldi r24 ,39
ldi r25 ,0
rcall wait_usec
ldi r24 ,0x20 ; a??a?? se 4-bit mode
out PORTD ,r24
sbi PORTD ,PD3
cbi PORTD ,PD3
ldi r24 ,39
ldi r25 ,0
rcall wait_usec
ldi r24 ,0x28 ; ep????? ?a?a?t???? �e?????? 5x8 ?????d??
rcall lcd_command ; ?a? e�f???s? d?? ??a��?? st?? ?????
ldi r24 ,0x0c ; e?e???p???s? t?? ??????, ap?????? t?? ???s??a
rcall lcd_command
ldi r24 ,0x01 ; ?a?a??s�?? t?? ??????
rcall lcd_command
ldi r24 ,low(1530)
ldi r25 ,high(1530)
rcall wait_usec
ldi r24 ,0x06 ; e?e???p???s? a?t?�at?? a???s?? ?at? 1 t?? d?e????s??
rcall lcd_command ; p?? e??a? ap????e?�??? st?? �et??t? d?e????se?? ?a?
; ape?e???p???s? t?? ???s??s?? ????????? t?? ??????
ret

scan_keypad_rising_edge:
mov r22 ,r24 		; ap????e?se t? ????? sp??????s�?? st?? r22
rcall scan_keypad ; ??e??e t? p???t??????? ??a p?es�????? d?a??pte?
push r24			 ; ?a? ap????e?se t? ap?t??es�a
push r25
mov r24 ,r22 		; ?a??st???se r22 ms (t?p???? t?�?? 10-20 msec p?? ?a?????eta? ap? t??
ldi r25 ,0			 ; ?atas?e?ast? t?? p???t???????? � ?????d????e?a sp??????s�??)
rcall wait_msec
rcall scan_keypad 	; ??e??e t? p???t??????? ?a?? ?a? ap?????e
pop r23 			; ?sa p???t?a e�fa?????? sp??????s�?
pop r22
and r24 ,r22
and r25 ,r23
ldi r26 ,low(_tmp_) ; f??t?se t?? ?at?stas? t?? d?a??pt?? st??
ldi r27 ,high(_tmp_) ; p??????�e?? ???s? t?? ???t??a? st??? r27:r26
ld r23 ,X+
ld r22 ,X
st X ,r24 			; ap????e?se st? RAM t? ??a ?at?stas?
st -X ,r25			 ; t?? d?a??pt??
com r23
com r22 			; ??e? t??? d?a??pte? p?? ????? ��????� pat??e?
and r24 ,r22
and r25 ,r23
ret

