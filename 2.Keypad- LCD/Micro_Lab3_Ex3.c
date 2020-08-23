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

int main(){
	DDRB=0xFF;
	char B0,B7;


	LCD_Init();
	char minH='0';
	char minL='0';
	char secH='0';
	char secL='0';
	LCD_Char(minH);
	LCD_Char(minL);
	LCD_String ("MIN:");
	LCD_Char(secH)	;
	LCD_Char(secL);
	LCD_String (" SEC");
	B0=PINB & 0x01;
	B7=PINB&0x80
	while(B0==0){
		B0=PINB & 0x01;
	}

	while(1){
		_delay_ms(10000) ;
		secL=secL+1;
		if(secL>'9'){
			secL='0';
			secH=secH+1;
			if(secH>='6'){
				secH='0';
				minL=minL+1;
				if(minL>'9'){
					minL='0';
					minH=minH+1;
					if(minH>='6'){
						minH='0';
						minL='0';
						secH='0';
						secL='0';
					}
				}
			}
		}
		LCD_Char(minH);
		LCD_Char(minL);
		LCD_String ("MIN:");
		LCD_Char(secH)	;
		LCD_Char(secL);
		LCD_String (" SEC");
		B7=PINB&0x80
		if(B7!=0){
			minH='0';
			minL='0';
			secH='0';
			secL='0';
		}
		while(B0==0){
			B0=PINB & 0x01;
		}
	}
}
