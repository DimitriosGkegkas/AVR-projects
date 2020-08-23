#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include inbuilt defined Delay header file */





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

void unlock(void){
	PORTB=0xFF;
	_delay_ms(10000) ;
		_delay_ms(10000) ;
			_delay_ms(10000) ;
			_delay_ms(10000) ;
	PORTB=0x00;
	return;
}
void blingLeds(void){
	
	for(int i=0;i<8;i++){
		PORTB=0xFF;
		_delay_ms(2500) ;
		PORTB=0x00;
		_delay_ms(2500) ;
	}
	return;
}
int main(void){
	char x;
    while (1) {
		x=keyfind();
		if(x==0x30){
			x=keyfind();
			if(x==0x37) unlock();
			else blingLeds();
		}
		else{
			x=keyfind();
			blingLeds();
		}
		
    }
}


