
//-------------------------------------------------------------------------
//----Get_temp function----------------------------------------------------
//----Returns the temperature or 8000 if no device is found----------------
//-------------------------------------------------------------------------
uint16_t get_temp(void){
    uint8_t dev=ds18b20_reset();		//reset and check if a device is connected
    if(dev==0x01){				        //if not return 0x8000
        uint16_t re=0x8000;
        return re;
    }
    ds18b20_writebyte(0xCC);		    //only one device
    ds18b20_writebyte(0x44);		    //begin reading temperature
    uint8_t flag=ds18b20_readbit();
    while(flag==1){
        flag=ds18b20_readbit();
    }

    dev=ds18b20_reset();			    //reset and check if a device is connected
    ds18b20_writebyte(0xCC);	    	//only one device
    ds18b20_writebyte(0xBE);		    //read the 16bit temperature
    uint8_t a= ds18b20_readbyte();
    uint8_t b=  ds18b20_readbyte();
    b=  ds18b20_readbyte();		        //and store it in registers r25:r24

    uint16_t re=0x0000;
    re=re+b;					        //result is in re
    re=re<<8;
    re=re+a;				            //result reformed to remove the fractional part
    re=re>>1;
    re=re & 0x7F;
    return re;
}
//-------------------------------------------------------------------------
//----Main Program---------------------------------------------------------
//----Just To check the get_temp function----------------------------------
//-------------------------------------------------------------------------
int main(){
    DDRB=0xFF;				             //PORTB as output
    while(1){
        uint16_t temp=get_temp();	    //get temperature
        if(temp==0x8000){		        //if no device is found display 0x01
            PORTB=0x01;
            continue;
        }
        PORTB=(uint8_t)temp;		    //display the result on PORTB
    }
}
