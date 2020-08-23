;---------------------------------------------------------------------------
;----Initialization---------------------------------------------------------	


#include "m16def.inc"
	.org 0x0
	rjmp reset
	.org 0x2
	rjmp ISR0
		
	
	reset:
		
	ldi	r16,LOW(RAMEND)		; load low byte of RAMEND into r16
	out	SPL,r16			; store r16 in stack pointer low
	ldi	r16,HIGH(RAMEND)	; load high byte of RAMEND into r16
	out	SPH,r16	
	ldi r24,(1<<ISC01)|(1<<ISC00)	;Interruption at Positive edge
	out MCUCR,r24

	ldi r24,(1<<INT0)		;enable INT0
	out GICR,r24
	sei
	
	clr r20
	ser r26 			;PortB as output
	out DDRB,r26 

	out DDRC,r26
	clr r26 			;PortD as Input
	out DDRD,r26 
	clr r26
	out DDRA,r26
;---------------------------------------------------------------------------
;----Main Program-----------------------------------------------------------
;----Counter At Leds--------------------------------------------------------
loop:		
	out PORTB,r20			;refresh leds with counter
	ldi r24,low(200)
	ldi r25,high(200)
	rcall wait_msec			;call wait_msec for Delay with input delay at r24-r25
	inc r20					;increase counter
	in r23,PIND	
	andi r23,0x80				
	cpi r23,0x80			;checks if PD7 is ON
	breq allow				;if yes allow interruptions
	ldi r24,(0<<INT0)		;else close interruptions
	out GICR,r24
	cli	
	rjmp loop				;start again
allow:
	ldi r24,(1<<INT0)					
	out GICR,r24
	sei
	rjmp loop 				;start again
;---------------------------------------------------------------------------
;----Interuption Routine----------------------------------------------------
;----Lights as many leds of PORTC as the switches of PORTA that are ON------
ISR0:
	ldi r24,(1<<INTF0)
	out GIFR,r24
	ldi r24,0b0101
	ldi r25,0b0000		;delay 50 ms
	rcall wait_msec
	in r24,GIFR
	rol r24
	rol r24
	brcs ISR0			;if yes then check again
	push r26			;else perform the interruption
	in r26,SREG			;saving important data
	push r26
	in r26,PINA
	clr r19				;initializing r20 and r21
	clr r21
	ldi r21,0b1001		;load 9 in r21 so that checkloop
checkLoop:
	dec r21				;decrease r21
	cpi r21,0x00
	breq endLoop		;if all 8 switches are checked go to endLoop
	rol r26				;check 
	brcc checkLoop		;if yes repeat the process
	rol r19				;else rotate the output and add a 1 at the LSB(it is the carry)
	rjmp checkLoop		;repeat the process
endLoop:
	out PORTC,r19		;show the output of PORTC 
	pop r26				;recovering data
	out SREG,r26
	pop r26
	reti
;---------------------------------------------------------------------------
;---------------------------------------------------------------------------	
wait_msec:
 push r24 
 push r25 
 ldi r24 , low(998) 
 ldi r25 , high(998) 
 rcall wait_usec 
 pop r25 
 pop r24 
 sbiw r24,1 
 brne wait_msec 
 ret 

 wait_usec:
sbiw r24,1 
nop 
nop 
nop 
nop
brne wait_usec 
ret 
end:
