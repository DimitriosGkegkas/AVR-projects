;---------------------------------------------------------------------------
;----Main Program-----------------------------------------------------------
;----Counter At Leds--------------------------------------------------------
ser r20		;r20 as flag for the interupt
loop:	
	cpi r20,0	;if flag=0 then we have to wait for another interupt 
	breq SET	;and we are gonna display 1

	in r22,PINB	;else we are reading the input
	mov r23,r22	;making two copies
	mov r24,r22
	lsl r22		;shifting one of the 3 left (r22)
	or r23,r22	;making the two first or gates
	andi r23,0b1010000
	and r24,r22	;making the two first and gates
	andi r24,0b0001010
	lsl r24
	lsl r24
	lsl r24
	lsl r24		;bring the two in illignment 
	or r23,r24	;and performing or in pears of 2
	mov r24,r23	;bring again in illignment 
	lsl r23
	lsl r23
	or r23,r24	;and finaly performing the last or
	andi r23,80H	;our result is in MSB of r23
	lsr r23
	lsr r23
	lsr r23
	lsr r23
	lsr r23
	lsr r23			
	lsr r23  
	andi r23,01H
	cpi r23,00H
	breq ZERO
	ldi r23,(1<<PC1)	;display one
	out PORTC,r23
	rjmp loop
ZERO:
	ldi r23,(0<<PC1)	;display one
	out PORTC,r23
	rjmp loop
	



	out PORTC,r23	;and exporting the result
	rjmp loop

SET:
	ldi r23,(1<<PC1)	;display one
	out PORTC,r23
	jmp loop

