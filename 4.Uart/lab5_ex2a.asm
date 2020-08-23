

#include "m16def.inc"

.DSEG
_tmp_:.byte 2
.CSEG

.org 0x00
rjmp reset
.org 0x1C
rjmp inter
reset:
	ldi	r16,LOW(RAMEND)		; load low byte of RAMEND into r16
	out	SPL,r16			; store r16 in stack pointer low
	ldi	r16,HIGH(RAMEND)	; load high byte of RAMEND into r16
	out	SPH,r16		
	ser r24
	out DDRB,r24
	ser r24
	out DDRC,r24
	call usart_init		;initialize
	rcall ADC_init
main:
	sei					;opening interrupts
	ldi r24,0xCF
	out ADCSRA, r24		;starting ADC
	clr r20				
	loop:				;starting the counter 
	ldi r25 ,low(200)	; wait 200 µs
	ldi r24 ,high(200)
	rcall wait_usec
	inc r20
	rjmp loop


inter:		
	in r22,ADCL			;reading result
	in r23,ADCH			;r22->LSB , r23->MSB
	andi r23,0x03		;mask r23 to only have the 2LSB
	;------------------------------------------------
	;----multiply r23:r22 *100-----------------------
	;----result in r26:r25:r24-----------------------
	ldi r21,0x64		
	mul r22,r21			
	mov r24,r0
	mov r25,r1
	mul r23,r21
	add r25,r0
	mov r26,r1			
	;------------------------------------------------
	;----div r26:r25:r24 /1024=2^10------------------
	;----result in r25-------------------------------
	lsr r25
	lsr r25
	lsl r26
	lsl r26
	lsl r26
	lsl r26
	lsl r26
	lsl r26
	add r25,r26
	;------------------------------------------------
	;----multiply r25 *5-----------------------
	;----result in r23:r22-----------------------
	ldi r21, 0x05
	mul r25,r21
	mov r22,r0
	mov r23,r1
	andi r23,0x03
	;------------------------------------------------
	clr r25			;hundreds
	clr r20			;dozens
	clr r18			;units

	cpi r23,0x00	;if r23=0
	breq next		;onlyt focus on r22
	ldi r25,0x02	;else you already have 2 hundreds
	ldi r20,0x05	;5 dozens
	ldi r18,0x06	;6 units from the 9th bit that was stored at r23
	next:
		cpi r22,0x64	;if r22 has no longer hundreds 
		brlo finH		;break from the loop
		inc r25			;else increase the hundreds
		subi r22,0x64	;and take from r22 1 hundred
		rjmp next		;go check again
		finH:
			cpi r22,0x0A	;if r22 has no longer dozens 
			brlo finD		;break from the loop
			inc r20			;else increase the dozens
			subi r22,0x0A	;and take from r22 1 dozen
			cpi r20,0x0A	;if dozens not exceed 9
			brne finH		;go check again
			inc r25			;else increase the hundreds
			ldi r20,0x00	;and start dozens from zero
			rjmp finH		;go check again
			finD:
				add r18,r22		;r22 is left only with units
				cpi r19,0x0A	;if units not exceed 9
				brlo print		;print the result
				inc r20			;else increase the dozens
				subi r18,0x0A	;and take from r18 1 dozen
				cpi r20,0x0A	;if dozens not exceed 9
				brne print		;print the result
				inc r25			;else increase the hundreds
				ldi r20,0x00	;and start dozens from zero
	;------------------------------------------------
	;----prints the result in the following format---
	;----r25 , r20 r18  V----------------------------
		print:
			mov r24,r25
			ldi r21,0x30
			add r24,r21
			call usart_transmit
			ldi r24,','
			call usart_transmit
			mov  r24,r20
			ldi r21,0x30
			add r24,r21
			call usart_transmit
			mov r24,r18
			ldi r21,0x30
			add r24,r21
			call usart_transmit
			ldi r24,' '
			call usart_transmit
			ldi r24,'\n'
			call usart_transmit
			rjmp main
	;------------------------------------------------

usart_init:
clr r24             ; initialize UCSRA to zero
out UCSRA ,r24
ldi r24 ,(1<<RXEN) | (1<<TXEN)        ; activate transmitter/receiver
out UCSRB ,r24       
ldi r24 ,0                            ; baud rate = 9600
out UBRRH ,r24
ldi r24 ,51
out UBRRL ,r24
ldi r24 ,(1 << URSEL) | (3 << UCSZ0)  ; 8-bit character size,
out UCSRC ,r24                        ; 1 stop bit
ret



usart_transmit:
sbis UCSRA ,UDRE       ; check if usart is ready to transmit
rjmp usart_transmit    ; if no check again, else transmit
out UDR ,r24           ; content of r24
ret


usart_receive: 
sbis UCSRA ,RXC ; check if usart received byte 
rjmp usart_receive ; if no check again, else read 
in r24 ,UDR ; receive byte and place it in 
ret ; r24


ADC_init:
clr r24
out DDRA,r24
ldi r24,0x40
out ADMUX, r24 
ldi r24,0x8F
out ADCSRA, r24
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
