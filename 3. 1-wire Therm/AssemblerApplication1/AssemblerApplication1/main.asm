;-------------------------------------------------------------------------
;----Main Program---------------------------------------------------------
;-------------------------------------------------------------------------
main:
call read_temp		;Reading Temperature
cpi r25,0x80
brne device
cpi r24,0x00		;If no device found
breq main		;go to main
device:
cpi r25,0xFF		;If MSBs are all 1
brne pos		
ror r24			;the temperature is negative,so rotate the number once
ori r24,0x80		;make MSB zero
rjmp export		;display the temperature in PORTB
pos:
ror r24			;else if positive, rotate the number once
andi r24,0x7F		;make MSB zero
export:			;display the temperature in PORTB
out PORTB,r24
rjmp main
;-------------------------------------------------------------------------
;----read_temp function----------------------------------------------------
;----Returns the temperature or 8000 if no device is found----------------
;-------------------------------------------------------------------------
read_temp:
call one_wire_reset		;reset and check if a device is connected
cpi r24,0x00
brne device_de			;if yes go to device_de(detected)
ldi r25, 0x80
ldi r24,0x00
ret				;if not return 0x800
device_de:
ldi r24,0xCC			;only one device
call one_wire_transmit_byte
ldi r24,0x44			;begin reading temperature
call one_wire_transmit_byte
wait:
call one_wire_receive_bit	
sbrs r24,0	
rjmp wait
call one_wire_reset		;reset and check if a device is connected
ldi r24,0xCC			;only one device
call one_wire_transmit_byte
ldi r24,0xBE			;read the 16bit temperature
call one_wire_transmit_byte

call one_wire_receive_byte	
mov r20,r24
call one_wire_receive_byte
mov r25,r24			;and store it in registers r25:r24
mov r24,r20
ret
