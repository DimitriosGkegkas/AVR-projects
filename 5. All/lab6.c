#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */

#define LCD_Dir  DDRD			/* Define LCD data port direction */
#define LCD_Port PORTD			/* Define LCD data port */
#define RS PD2			/* Define Register Select pin */
#define EN PD3 	



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
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(480);

	//release line and wait for 60uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(60);

	//get value and wait 420us
	i = (DS18B20_PIN & (1<<DS18B20_DQ));
	_delay_us(420);

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
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec

/*
 * get temperature
 */
double get_temp() {
	uint8_t temperature_l;
	uint8_t temperature_h;
	double retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP); //start temperature conversion

	while(!ds18b20_readbit()); //wait until conversion is complete

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 2 byte from scratchpad
	temperature_l = ds18b20_readbyte();
	temperature_h = ds18b20_readbyte();

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	//convert the 12 bit value obtained
	retd = ( ( temperature_h << 8 ) + temperature_l ) * 0.0625;

	return retd;
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
	LCD_Dir = 0xFC
	;			/* Make LCD port direction as o/p */
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
	for(i=0;str[i]!='\0';i++)		/* Send each char of string till the NULL */
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


char scan_row_new(int row){
	char p=0x08;
	p=p<<row;
	PORTC=p;
	_delay_ms(2);
	char ret = PINC;
	ret=ret&0x0F;
	return ret;
}

uint16_t scan_keypad_new(){

	DDRC=(1 << PC7) | (1 << PC6) | (1 << PC5) | (1 << PC4)  ;
	uint16_t a=scan_row_new(1);
	a=a<<12;
	uint16_t b=scan_row_new(2);
	b=b<<8;
	a=b+a;
	uint16_t c=scan_row_new(3);
	c=c<<4;
	a=a+c;
	uint16_t d=scan_row_new(4);
	a=a+d;
	return a;
}


#define KEY_PRT 	PORTC
#define KEY_DDR		DDRC
#define KEY_PIN		PINC

unsigned char keypad[4][4] = {	{'7','8','9','/'},
				{'4','5','6','*'},
				{'1','2','3','-'},
				{' ','0','=','+'}};

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
			if(colloc!=0x0F) return 0x00;
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


int main(){
	LCD_Init();		//initialize lcd screen
	DDRB=0xFF;		//set PORTB as output
	usart_init();		//initialize usart
		char message[] = "teamname: " ; 
		int i=0;
		while(message[i]!='\0'){
			usart_transmit(message[i]);	//show teamname on usart
			i++;		
		}
		usart_transmit('"');		//show "C1" on usart
		usart_transmit('C');
		usart_transmit('1');
		usart_transmit('"');	
		usart_transmit('\n');	

		char rec[32];
		rec[0]=usart_receive();

		i=0;
		while(rec[i]!='\n'){
			i++;
			rec[i]=usart_receive();	//receive answer Success or Fail
		}							//from ESP8266 


		LCD_Init();
		LCD_String ("1.");
		i=0;
		while(rec[i]!='\n'){
			LCD_Char(rec[i]);		//show on LCD 1.(answer)
			i++;
		}
		_delay_ms(1000);

		char message2[] = "connect" ;
connectt:
		i=0;
		while(message2[i]!='\0'){
			usart_transmit(message2[i]);	//show connect on usart
			i++;		
		}	
		usart_transmit('\n');	

		rec[0]=usart_receive();

		i=0;
		while(rec[i]!='\n'){
			i++;
			rec[i]=usart_receive();//receive answer Success or Fail
									//from ESP8266 

		}
		if(rec[1]=='F') goto connectt;	//if Fail, repeat connection

	
		LCD_Init();
		LCD_String ("2.");
		i=0;
		while(rec[i]!='\n'){
			LCD_Char(rec[i]);		//show on LCD 2."Success"
			i++;
		}
		_delay_ms(1000);


	while(1){
		int res=0;
		int de=0;
		int ek=0;
		uint16_t temp=get_temp();	//Reading temperature
		while(temp==0x8000){
			temp=get_temp();	//While no device read again
		}
		
			res=(uint8_t)temp;	//res now has the value of the Temp

			if(temp>=0x8000) {	 //if it is negative 
				res=res-1;
				res= ~res;	//we take its positive compliment
			}
			else{
			}
		 	ek= res/0x64;	 //Checking for the hundreds 
			res=res-ek*100;
			de=res/0x0A;	 //then for the dozens 
			res=res-de*10;
	
		

		_delay_ms(1000);
		uint16_t c=scan_keypad_new();	//read from keypad

		if(c==0x1000){
			char message3[] = "ready: " ;	//if the number pressed is 1
			i=0;
			while(message3[i]!='\0'){
				usart_transmit(message3[i]);	//show "ready:" on usart screen
				i++;		
			}	
			usart_transmit('"');
			char message4[] = "true" ;		
			i=0;
			while(message4[i]!='\0'){
				usart_transmit(message4[i]);	//show "true"
				i++;		
			}	
			usart_transmit('"');
			usart_transmit('\n');
			rec[0]=usart_receive();


			i=0;
			while(rec[i]!='\n'){
				i++;
				rec[i]=usart_receive();	//receive answer Success or Fail
			}							//from ESP8266

	
			LCD_Init();
			LCD_String ("4.");
			i=0;
			while(rec[i]!='\n'){
				LCD_Char(rec[i]);		//show on LCD 4.(answer)
				i++;
			}

			_delay_ms(1000);	
		}
		else if (c!=0x0000){
			char message10[] = "ready: " 	//else if something else is pressed
			i=0;
			while(message10[i]!='\0'){
				usart_transmit(message10[i]);	//show "ready:false" on usart screen
				i++;		
			}	
			usart_transmit('"');
			char message11[] = "false" ;
			i=0;
			while(message11[i]!='\0'){
				usart_transmit(message11[i]);
				i++;		
			}	
			usart_transmit('"');
			usart_transmit('\n');
			rec[0]=usart_receive();


			i=0;
			while(rec[i]!='\n'){
				i++;
				rec[i]=usart_receive();//receive answer Success or Fail
										//from ESP8266
			}

	
			LCD_Init();
			LCD_String ("4.");
			i=0;
			while(rec[i]!='\n'){
				LCD_Char(rec[i]);		//show on LCD 4.(answer)
				i++;
			}

			_delay_ms(1000);
		
		}


		char message5[] = "payload:[{" ;
			i=0;
			while(message5[i]!='\0'){
				usart_transmit(message5[i]);	//show "payload: [{"name": "Temperature","value" , on uart
				i++;		
			}	
			usart_transmit('"');
			char message7[] = "name" ;
			i=0;
			while(message7[i]!='\0'){
				usart_transmit(message7[i]);
				i++;		
			}
			usart_transmit('"');
			usart_transmit(':');
			usart_transmit('"');
			char message6[] = "Temperature" ;
			i=0;
			while(message6[i]!='\0'){
				usart_transmit(message6[i]);
				i++;		
			}	
			usart_transmit('"');
			usart_transmit(',');
			usart_transmit('"');
			char message8[] = "value" ;
			i=0;
			while(message8[i]!='\0'){
				usart_transmit(message8[i]);
				i++;		
			}
			usart_transmit('"');
			usart_transmit(':');
			
			if(ek!=0) usart_transmit(ek+0x30);	//show temperature in decimal on uart
			usart_transmit(de+0x30);
			usart_transmit(res+0x30);
			usart_transmit('}');
			usart_transmit(']');
			usart_transmit('\n');


					rec[0]=usart_receive();

		i=0;
		while(rec[i]!='\n'){
			i++;
			rec[i]=usart_receive();		//receive answer Success or Fail
										//from ESP8266

		}
		LCD_Init();
		LCD_String ("3.");
		i=0;
		while(rec[i]!='\n'){
			LCD_Char(rec[i]);			//show on LCD 3.(answer)
			i++;
		}


		_delay_ms(1000);
			char message9[] = "transmit" ;
			i=0;
			while(message9[i]!='\0'){
				usart_transmit(message9[i]);	//show "transmit" on uart
				i++;		
			}
			usart_transmit('\n');	


			
	rec[0]=usart_receive();
		i=0;
		while(rec[i]!='\n'){
			i++;
			rec[i]=usart_receive();		//receive answer Success or Fail
										//from ESP8266

		}
		LCD_Init();
		LCD_String ("5.");
		i=0;
		while(rec[i]!='\n'){
			LCD_Char(rec[i]);		//show on LCD 5.(answer)
			i++;
		}
	}
	}

