/*  
   LCD16x2 4 bit ATmega16 interface
   http://www.electronicwings.com
*/



#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */

#define LCD_Dir  DDRD			/* Define LCD data port direction */
#define LCD_Port PORTD			/* Define LCD data port */
#define RS PD2			/* Define Register Select pin */
#define EN PD3 				/* Define Enable signal pin */


#include "ds18b20.h"

#define DS18B20_PORT PORTA
#define DS18B20_DDR DDRA
#define DS18B20_PIN PINA
#define DS18B20_DQ PC4
 
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




void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		/* RS=0, command reg. */
	LCD_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);		/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			/* Make LCD port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command(0x02);		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/
	_delay_ms(2);
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}
 #define KEY_PRT 	PORTC
#define KEY_DDR		DDRC
#define KEY_PIN		PINC

unsigned char keypad[4][4] = {	{'1','2','3','A'},
				{'4','5','6','B'},
				{'7','8','9','C'},
				{'E','0','F','D'}};

unsigned char colloc, rowloc;
char keyfind()
{
	while(1)
	{
	    KEY_DDR = 0xF0;           /* set port direction as input-output */
	    KEY_PRT = 0xFF;

	    do
	    {

		KEY_PRT &= 0x0F;      /* mask PORT for column read only */
		asm("NOP");
		colloc = (KEY_PIN & 0x0F); /* read status of column */
	    }while(colloc != 0x0F);
		
	    do
	    {
		do
		{
	            _delay_ms(20);             /* 20ms key debounce time */
		    colloc = (KEY_PIN & 0x0F); /* read status of column */
		}while(colloc == 0x0F);        /* check for any key press */
			
		_delay_ms (40);	            /* 20 ms key debounce time */
		colloc = (KEY_PIN & 0x0F);
	    }while(colloc == 0x0F);

	   /* now check for rows */
	    KEY_PRT = 0xEF;            /* check for pressed key in 1st row */
	    asm("NOP");
	    colloc = (KEY_PIN & 0x0F);
	    if(colloc != 0x0F)
            {
		rowloc = 0;
		break;
	    }

	    KEY_PRT = 0xDF;		/* check for pressed key in 2nd row */
	    asm("NOP");
	    colloc = (KEY_PIN & 0x0F);
	    if(colloc != 0x0F)
	    {
		rowloc = 1;
		break;
	    }
		
	    KEY_PRT = 0xBF;		/* check for pressed key in 3rd row */
	    asm("NOP");
            colloc = (KEY_PIN & 0x0F);
	    if(colloc != 0x0F)
	    {
		rowloc = 2;
		break;
	    }

	    KEY_PRT = 0x7F;		/* check for pressed key in 4th row */
	    asm("NOP");
	    colloc = (KEY_PIN & 0x0F);
	    if(colloc != 0x0F)
	    {
		rowloc = 3;
		break;
	    }
	}

	if(colloc == 0x0E)
	   return(keypad[rowloc][0]);
	else if(colloc == 0x0D)
	   return(keypad[rowloc][1]);
	else if(colloc == 0x0B)
	   return(keypad[rowloc][2]);
	else
	   return(keypad[rowloc][3]);
}   

uint16_t get_temp(void){
    uint8_t dev=ds18b20_reset();
    if(dev==0x01){
        uint16_t re=0x8000;
        return re;
    }
    ds18b20_writebyte(0xCC);
    ds18b20_writebyte(0x44);
    uint8_t flag=ds18b20_readbit();
    while(flag==1){
        flag=ds18b20_readbit();
    }

    dev=ds18b20_reset();
    ds18b20_writebyte(0xCC);
    ds18b20_writebyte(0xBE);
    uint8_t a= ds18b20_readbyte();
    uint8_t b=  ds18b20_readbyte();
    b=  ds18b20_readbyte();

    uint16_t re=0x0000;
    re=re+b;
    re=re<<8;
    re=re+a;
    re=re>>1;
    re=re & 0x7F;
    return re;
}


int main(){
	while(1){
		LCD_Init();
		uint16_t temp=get_temp();					//Reading Temperature
		if(temp==0x8000) LCD_String ("No Device");	//If no device found print "No Device"
		else{
			uint8_t res=(uint8_t)temp;				//res now has the value of the Temp.
			if(temp>=0x8000) {						//if it is negative
				LCD_Char('-');						//we print a '-'
				res=res-1;
				res= ~res;							//and take its positive compliment
			}
			else LCD_Char('+');						//else we just print '+'
			res=res>>1;								//Now we have to print the value with out the decimal
			int ek= res/0x64;						//Checking for the hundreds
			res=res-ek*100;		
			int de=res/0x0A;						//then for the dozens
			res=res-de*10;
			if(ek!=0) LCD_Char(ek+0x30);			//and we just print the result
			LCD_Char(de+0x30);
			LCD_Char(res+0x30);
			LCD_Char('�');
			LCD_Char('C');
		}
	}	
}
