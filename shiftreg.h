/*
 * IncFile1.h
 *
 * Created: 10/17/2016 10:37:25 AM
 *  Author: jesselayman
 */ 


#ifndef SHIFTREG_H_
#define SHIFTREG




void shiftRegInit(){
	// Ensure DDRC is setup as output
	DDRC = 0xFF; PORTC = 0x00;
}


void transmit_ucdata(unsigned char data) {
	unsigned char i=8;
	unsigned char SER;
	// SER = DDRC0, SRCLK = DDRC1, RCLK = DDRC2
	// SRCLR = DDRC3
	unsigned char SRCLR = 0x08;
	unsigned char RCLK = 0x02;
	unsigned char SRCLK = 0x04;
	
	//before transmit set SRCLR high
	PORTC = SRCLR;
	while(i>0){
	// set SER = next bit of data to be sent.


	   SER = (data >> i) & 0x01;
	   	// for each bit of data set SRCLK low, SER to bitval
		PORTC = SER|SRCLR;//SER;
	// one bit sent, set SRCLK high
		PORTC = SRCLK|SRCLR;
		i--;
		if (i==0){
		   SER = data&0x01;
		   // for each bit of data set SRCLK low, SER to bitval
		   PORTC = SER|SRCLR;//SER;
		   // one bit sent, set SRCLK high
		   PORTC = SRCLK|SRCLR;}
	}
	
	//send bit 0x01;
	
	// Data serialized, set RCLK HIGH, OE LOW
	
		PORTC = SRCLK|RCLK|SRCLR;
		
		PORTC = 0x00;
	}
void transmit_usdata(unsigned short data) {
	unsigned char i=14;
	unsigned char SER;
	// SER = DDRC0, SRCLK = DDRC1, RCLK = DDRC2
	// SRCLR = DDRC3
	unsigned char SRCLR = 0x08;
	unsigned char RCLK = 0x02;
	unsigned char SRCLK = 0x04;
	
	//before transmit set SRCLR high
	PORTC = SRCLR;
	while(i>0){
		// set SER = next bit of data to be sent.


		SER = (data >> i) & 0x01;
		// for each bit of data set SRCLK low, SER to bitval
		PORTC = SER|SRCLR;//SER;
		// one bit sent, set SRCLK high
		PORTC = SRCLK|SRCLR;
		i--;
		if (i==0){
			SER = data&0x01;
			// for each bit of data set SRCLK low, SER to bitval
			PORTC = SER|SRCLR;//SER;
			// one bit sent, set SRCLK high
		PORTC = SRCLK|SRCLR;}
	}
	
	//send bit 0x01;
	
	// Data serialized, set RCLK HIGH, OE LOW
	
	PORTC = SRCLK|RCLK|SRCLR;
	
	PORTC = 0x00;
}
	
	void transmit_data2(unsigned char data) {
		unsigned char i=8;
		unsigned char SER;
		// SER = DDRC0, RCLK = DDRC1, SRCLK = DDRC2
		// SRCLR = DDRC3
		unsigned char SRCLR = 0x80;
		unsigned char RCLK = 0x20;
		unsigned char SRCLK = 0x40;
		
		//before transmit set SRCLR high
		PORTC = SRCLR;
		while(i>0){
			// set SER = next bit of data to be sent.


			if((data >> i) & 0x01){
				SER = 0x10;
			}
			else {SER = 0x00;}
			// for each bit of data set SRCLK low, SER to bitval
			PORTC = SER|SRCLR;//SER;
			// one bit sent, set SRCLK high
			PORTC = SRCLK|SRCLR;
			i--;
			if (i==0){
				if (data&0x01)
				{
					SER = 0x10;
				}
				else{
				SER = data&0x01;}
				// for each bit of data set SRCLK low, SER to bitval
				PORTC = SER|SRCLR;//SER;
				// one bit sent, set SRCLK high
			PORTC = SRCLK|SRCLR;}
		}
		
		//send bit 0x01;
		
		// Data serialized, set RCLK HIGH, OE LOW
		
		PORTC = SRCLK|RCLK|SRCLR;
		
		PORTC = 0x00;
		
		
		
		
	}
	
	
	





#endif /* SHIFTREG */