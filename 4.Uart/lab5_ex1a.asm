
#include "m16def.inc"

.DSEG
_tmp_:.byte 2
.CSEG

.org 0x0
rjmp reset

reset:
	ldi	r16,LOW(RAMEND)		; load low byte of RAMEND into r16
	out	SPL,r16			; store r16 in stack pointer low
	ldi	r16,HIGH(RAMEND)	; load high byte of RAMEND into r16
	out	SPH,r16		
	ser r24
	out DDRB,r24
string: .DB 'H', 'e','l','l','o',' ', 'W', 'o','r','l','d','\n','\0'
call usart_init
main:


ldi ZH, high(string<<1); Initialize Z-pointer
ldi ZL, low(string<<1)

loop:
lpm r16, Z+ ; Load constant from Program
cpi r16,'\0'
breq main
cpi r16,0x00
breq loop
mov r24,r16
call usart_transmit

rjmp loop




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
