;---------------------------------------------------------------------------
;----Initialization---------------------------------------------------------


#include "m16def.inc"
	.org 0x0
	rjmp reset
	.org 0x4
	rjmp ISR1
	
	reset:
	
	ldi	r16,LOW(RAMEND)		; load low byte of RAMEND into r16
	out	SPL,r16			; store r16 in stack pointer low
	ldi	r16,HIGH(RAMEND)	; load high byte of RAMEND into r16
	out	SPH,r16		
	ldi r24,(1<<ISC11)|(1<<ISC10)	;Interruption at Positve edge
	out MCUCR,r24
	
	ldi r24,(1<<INT1)				;enable INT1
	out GICR,r24
	sei
	
	ser r26							;PortB as output
	out DDRB,r26 	
	out DDRA,r26
	
	clr r26	
	clr r20						;PortD as Input
	out DDRD,r26 
	out PORTA,r26
;---------------------------------------------------------------------------
;----Main Program-----------------------------------------------------------
;----Counter At Leds--------------------------------------------------------
loop:		
						;r26 is our counter
	out PORTB,r26		;refresh leds with counter
	ldi r24,low(200)
	ldi r25,high(200)
	rcall wait_msec		;call wait_msec for Delay with input delay at r24-r25
	inc r26				;increase counter
	in r23,PIND	
	andi r23,0x80				
	cpi r23,0x80		;checks if PD7 is ON
	breq allow			;if yes allow interruptions
	ldi r24,(0<<INT1)	;else close interruptions
	out GICR,r24
	cli	
	rjmp loop			;start again
allow:
	ldi r24,(1<<INT1)				
	out GICR,r24
	sei
	rjmp loop 			;start again
;---------------------------------------------------------------------------
;----Interuption Routine----------------------------------------------------
;----Shows the number of interruptions that have occured--------------------
ISR1:
	ldi r24,(1<<INTF1)
	out GIFR,r24

	ldi r24,0b0101					
	ldi r25,0b0000		;delay 50 ms
	rcall wait_msec
	in r24,GIFR
	rol r24
	brcs ISR1			;if yes then check again
	push r26			;else perform the interruption
	in r26,SREG			;saving importaned data
	push r26					
	inc r20				;increase the counter
	out PORTA,r20		;show the value of the counter on the leds
	pop r26				;recovering data
	out SREG,r26
	pop r26
	reti
;---------------------------------------------------------------------------
;---------------------------------------------------------------------------
wait_msec:			;	
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
