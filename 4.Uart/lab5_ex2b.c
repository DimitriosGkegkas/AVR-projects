/***************************************************************************************/
#define F_CPU 8000000UL
#include<avr/io.h>
#include<util/delay.h>
 
void adc_init(){
 	DDRA = 0X00; 
	ADCSRA = 0X8F;
	 ADMUX=0x40; 

}

void usart_init(void){
	UCSRA=0x00;
	UCSRB=(1<<RXEN)|(1<<TXEN);
	UBRRH=0x00;
	UBRRL=0x33;
	UCSRC=(1 << URSEL) | (3 << UCSZ0) ;
	return;
}


void usart_transmit(char p){
	char key= UCSRA & 0x20;
	while(key!=0x20){key= UCSRA & 0x20;}
	UDR=p;
	return;
}

char usart_receive(void){
	char key= UCSRA & 0x80;
	while(key!=0x80){
		key= UCSRA & 0x80;
		continue;}	
	char p=UDR;
	return p;
}



int main(){
adc_init();  
while(1){
	uint32_t b;
	uint32_t ad=0;
	ADCSRA=ADCSRA|0X40;                  /* ADSC=1,start conversion                      */
	while((ADCSRA & 0x40) == 0x40);      /* checking flag                                */
	b = ADCL;                            /* reading LSB result                           */
	ad = ADCH;                           /* reading MSB result                           */
	ad = ad<<8;                          /* shift MSB  8 times                           */
 	ad = ad|b;							 /* ad now has all the result from the ADC      */

	ad=ad*500;
	ad=ad/1024;							/* We now have the value of the volts mul by 100 */

	int hun=ad/100;						/* All we have to do is print the result in  */
	ad=ad-100*hun;						/* the following format   " hun  ','  de  mo  'V' " */
	int de=ad/10;						/* hun representing the hundreds of ad*/
	ad=ad-de*10;						/* de representing the dozens of ad*/
	
	usart_init(); 						/* mo representing the units of ad*/
	usart_transmit(hun+0x30);
	usart_transmit(',');
	usart_transmit(de+0x30);
	usart_transmit(ad+0x30);
	usart_transmit(' ');	
	usart_transmit('V');
	usart_transmit('\n');
	}               
}
