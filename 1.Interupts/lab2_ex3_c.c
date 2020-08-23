/***************************************************************************************/
#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
 #include<avr/interrupt.h>
	

ISR(INT1_vect){
	PORTB|=0xFF;
	TCNT1H|=0x85;
	TCNT1L|=0xEE;
	sei();
	_delay_ms(500);
	PORTB=0x01;
	return;
}


ISR(TIMER1_OVF_vect){
	PORTB=0x00;
	return;
}


int main(){
MCUCR|=(1<<ISC11)|(1<<ISC10)	;	
TIMSK|=(1<<TOIE1)	;
TCCR1B|=(1<<CS12)|(0<<CS11)|(1<<CS10);
GICR|=(1<<INT1);
DDRB|=0xFF;
DDRA|=0x00;
DDRD|=0x00;
sei();

	while(1){
		char x= PINA;
		x=x&0x80;
		if(x!=0x80) continue;
		PORTB=0xFF;
		_delay_ms(500);
		TCNT1H|=0x85;
		TCNT1L|=0xEE;
		
		PORTB=0x01;
	}
}



