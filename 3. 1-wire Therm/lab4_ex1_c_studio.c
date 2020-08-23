/*  
   LCD16x2 4 bit ATmega16 interface
   http://www.electronicwings.com
*/
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ds18b20.h"

 
/*
 * ds18b20 init
 */
uint8_t ds18b20_reset() {
	uint8_t i;
	
	//low for 480us
	DDRA |= (1<<PA4); //output
	PORTA &= ~ (1<<PA4); //low

	_delay_us(480);

	//release line and wait for 60uS
	DDRA &= ~(1<<PA4); //input
	PORTA &= ~(1<<PA4); //output
	_delay_us(100);

	//get value and wait 420us
	i = (PINA & (1<<PA4));
	_delay_us(380);




	//return the read value, 0=ok, 1=error
	return i;
}

/*
 * write one bit
 */
void ds18b20_writebit(uint8_t bit){
	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		DS18B20_DDR &= ~(1<<DS18B20_DQ); //input

	//wait 60uS and release the line
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
}

/*
 * read one bit
 */
uint8_t ds18b20_readbit(void){
	uint8_t bit=0;

	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//release line and wait for 14uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(14);

	//read the value
	if(DS18B20_PIN & (1<<DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

/*
 * write one byte
 */
void ds18b20_writebyte(uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}

/*
 * read one byte
 */
uint8_t ds18b20_readbyte(void){
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit()<<7);
	}
	return n;
}


uint16_t get_temp(void){
	uint8_t dev=ds18b20_reset();
	if(dev==0x01){
		uint16_t re=0x8000;
		return re;
	}
	ds18b20_writebyte(0xCC);
	ds18b20_writebyte(0x44);
	uint8_t flag=ds18b20_readbit();
	while(flag==1){
		flag=ds18b20_readbit();
	}

	dev=ds18b20_reset();
	ds18b20_writebyte(0xCC);
	ds18b20_writebyte(0xBE);
	uint8_t a= ds18b20_readbyte();
	uint8_t b=  ds18b20_readbyte();
 	b=  ds18b20_readbyte();

	uint16_t re=0x0000;
	re=re+b;
	re=re<<8;
	re=re+a;
	re=re>>1;
	re=re & 0x7F;
	return re;
}



int main(){
	DDRB=0xFF;
	while(1){
		uint16_t temp=get_temp();
		if(temp==0x8000){
			PORTB=0x01;
			continue;
		}
		PORTB=(uint8_t)temp;
	}
}
