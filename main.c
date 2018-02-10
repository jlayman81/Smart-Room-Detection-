/*	Partner(s) Name & E-mail: Jesse Layman, jlaym003@ucr.edu
 *	Lab Section: 021
 *	Assignment: FINAL PROJECT
 *
 *	I acknowledge all content contained herein, excluding template, example
 *	code, and reused code from previous classes authored by me, is my own original work.
 */




// Motion detectors entrySense and occupancySense are placed in series.
// PIR1 is closest to the door

#include <stdint.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <string.h> 
#include <math.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/eeprom.h> 
#include <avr/portpins.h> 
#include <avr/pgmspace.h> 

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "keypad.h"
#include "usart_ATmega1284.h"

unsigned char occupancyCount;
unsigned char checkOccupancyINC;
unsigned char checkOccupancyDEC;
unsigned char validateDEC;
unsigned char validateINC;
unsigned char light_on;
unsigned char dimmer;
unsigned char isDark;
unsigned char isHot;
unsigned char adcFlip;
unsigned char ON;

void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	DDRA = 0x00; PORTA = 0xFF;
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}
void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("NOP"); }
}

// ENTER_SM is used to poll the Entry motion detector 
enum ENTER_SM { es_init, es_poll, es_validate,es_stop } es_state;

 void ENTER_Init(){
	 es_state = es_init;
 }

 void ENTER_Tick(){
	 static unsigned char entryDetected;
	 static unsigned char i;
	 //Actions
	 switch(es_state){
		 case es_init:
		 break;
		 // entrySense is wired to B0
		 case es_poll:
		 entryDetected = PINB & 0x01;
		 break;		 
		 case es_validate:
		 checkOccupancyINC = 1;
		 break;	
		 case es_stop:
		 break;	 
	 }
	 //Transitions
	 switch(es_state){
		 case es_init:
		 entryDetected = 0;
		 validateINC = 0;
		 i = 0;
		 checkOccupancyINC = 0;
		 es_state = es_poll;
		 break;
		 // if entry is detected, validate
		 case es_poll:
		 if (entryDetected&&(validateDEC ==0))
		 {
			validateINC = 1;
			es_state = es_validate;
			break;
		 }
		 if ((entryDetected==0)&&(validateDEC ==0)){
		 es_state = es_poll;
		 break;
		 }
		 else{
			 es_state = es_stop;
			 break;
		 }
		 case es_stop:
		 if (validateDEC)
		 {
			 es_state = es_stop;
			 break;
		 }
		 else{
			 es_state = es_poll;
			 break;
		 }
		 case es_validate:
		 if (i < 40)
		 {
			 i++;
			 es_state = es_validate;
			 break;
		 }
		 else{
		 i = 0;
		 checkOccupancyINC = 0;
		 validateINC = 0;
		 es_state = es_poll;
		 break;	}	 
	 }
 }

 void ENTER_Task()
 {
	 ENTER_Init();

		 
	 for(;;)
	 {
		 if(ON){
		 ENTER_Tick();}
		 vTaskDelay(100);
	 }
	
 }

 void ENTER_Pulse(unsigned portBASE_TYPE Priority)
 {
	 xTaskCreate(ENTER_Task, (signed portCHAR *)"ENTER_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
 }
 
 // Occupant_Index uses pattern recognition to validate if a person has entered or exited the room
 enum OCCUPANT_INDEX_SM { oci_init, oci_wait, oci_dec, oci_inc } oci_state;

 void OCI_Init(){
	 oci_state = oci_init;
 }

 void OCI_Tick(){
	 static unsigned char occupied;
	 static unsigned char PIR;
	 static unsigned short i;
	 //Actions
	 switch(oci_state){
		 case oci_init:
		 break;
		 case oci_wait:
		 break;
		 case oci_dec:
		 PIR = PINB & 0x01;
		 break;
		 case oci_inc:
		 PIR = PINB & 0x02;
		 break;
		 
	 }
	 //Transitions
	 switch(oci_state){
		 case oci_init:
		 occupied = 0;
		 PIR = 0;
		 i=0;
		 oci_state = oci_wait;
		 occupancyCount = 0;// change to 0
		 break;
		 case oci_wait:
		 if ((checkOccupancyINC==1)&&(checkOccupancyDEC==0))
		 {
			 oci_state = oci_inc;
			 break;
		 }
		 if ((checkOccupancyINC==0)&&(checkOccupancyDEC==1))
		 {
			 oci_state = oci_dec;
			 break;
		 }
		 else{
			 oci_state = oci_wait;
		 break;}
		 case oci_dec:
		 if (i<400)
		 {
			 i++;
			 if (PIR)
			 {
				 occupied = 1;
				 PIR = 0;
				// i = 300;
			 }

			 oci_state = oci_dec;
			 break;
		 }
		 else{oci_state = oci_wait;
			 i = 0;
			 if (occupied)
			 {
				 if(occupancyCount > 0){
					 occupancyCount --;
				 }
				 
			 }
			 occupied = 0;
			 PIR = 0;
			 
		 break;} // NOW HERE
		 case oci_inc:
		 if (i<400)
		 {
			 i++;
			 if (PIR)
			 {
				 occupied = 1;
				 PIR = 0;
				// i = 300;
				 
			 }
			 oci_state = oci_inc;
			 break;
		 }
		 else{oci_state = oci_wait;
			 i = 0;
			 if (occupied)
			 {
				 occupancyCount ++;
			 }
			 occupied = 0;
			 PIR = 0;
		 break;}
	 }//HERE
 }

 void OCI_Task()
 {
	 OCI_Init();
	 for(;;)
	 {
		 OCI_Tick();
		 vTaskDelay(10);
	 }
 }

 void OCI_Pulse(unsigned portBASE_TYPE Priority)
 {
	 xTaskCreate(OCI_Task, (signed portCHAR *)"OCI_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
 }

 
 // Exit_SM is used to poll the occupancySense motion detector
 enum EXIT_SM { exs_init, exs_poll, exs_validate, exs_stop } exs_state;

 void EXIT_Init(){
	 exs_state = exs_init;
 }

 void EXIT_Tick(){
	 static unsigned char exitDetected;
	 static unsigned char i;
	 //Actions
	 switch(exs_state){
		 case exs_init:
		 break;
		 // occupancySense is wired to B1
		 case exs_poll:
		 exitDetected = PINB & 0x02;
		 break;
		 case exs_validate:
		 checkOccupancyDEC = 1;
		 break;
	 }
	 //Transitions
	 switch(exs_state){
		 case exs_init:
		 exitDetected = 0;
		 i = 0;
		 checkOccupancyDEC = 0;
		 validateDEC =0;
		 exs_state = exs_poll;
		 break;
		 // if exit is detected, validate
		 case exs_poll:
		 if (exitDetected&&(validateINC ==0))
		 {
			 validateDEC = 1;
			 exs_state = exs_validate;
			 break;
		 }
		 if ((exitDetected==0)&&(validateINC ==0)){
			 exs_state = exs_poll;
			 break;
		 }
		 else{
			 exs_state = exs_stop;
			 break;
		 }
		 case exs_stop:
		 if (validateINC)
		 {
			 exs_state = exs_stop;
			 break;
		 }
		 else{
			 exs_state = exs_poll;
			 break;
		 }
		 case exs_validate:
		 if (i < 20)
		 {
			 i++;
			 exs_state = exs_validate;
			 break;
		 }
		 else{
		 i = 0;
		 checkOccupancyDEC = 0;
		 exs_state = exs_poll;
		 validateDEC = 0;
		 break;}
	 }
 }

 void EXIT_Task()
 {
	 EXIT_Init();

	 for(;;)
	 {
		 if(ON){
		 EXIT_Tick();}
		 vTaskDelay(200);
	 }
 }

 void EXIT_Pulse(unsigned portBASE_TYPE Priority)
 {
	 xTaskCreate(EXIT_Task, (signed portCHAR *)"EXIT_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
 }
 // SM used to OUTPUT
  enum OUTPUT_SM { out_init, out_send } out_state;

  void OUT_Init(){
	  out_state = out_init;
  }

  void OUT_Tick(){
	  static unsigned char OUTPUT;
	  //Actions
	  switch(out_state){
		  case out_init:
		  break;
		  case out_send:
		  OUTPUT = ((~ON & 0x01)<<4)|(isDark <<5)|(isHot << 6) | occupancyCount | (light_on << 7);
		  break;
	  }
	  //Transitions
	  switch(out_state){
		  case out_init:
		  OUTPUT = 0;
		  out_state = out_send;
		  break;
		  case out_send:
		  PORTC = OUTPUT;
		  out_state = out_send;
		  break;
	  }
  }

  void OUT_Task()
  {
	  OUT_Init();
	  for(;;)
	  {
		  OUT_Tick();
		  vTaskDelay(10);
	  }
  }

  void OUT_Pulse(unsigned portBASE_TYPE Priority)
  {
	  xTaskCreate(OUT_Task, (signed portCHAR *)"OUT_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
  }
  
  enum LeaderState {INIT,waitTransmit,transmit} Leader_State;

  void Leader_Init(){
	  Leader_State = INIT;
  }

  void Leader_State_Tick(){
	  //Actions
	  static unsigned char ucTempOut;
	  static unsigned short stabilization;
	  static unsigned short i;
	  static unsigned char previous;
	  switch(Leader_State){
		  case INIT:
		  break;
		  case waitTransmit:
		  
		  break;
		  case transmit:
		  break;
		  
		  default:
		  
		  break;
	  }
	  //Transitions
	  switch(Leader_State){
		  case INIT:
		  //PORTD = 0xFF;
		  initUSART(0);
		  ucTempOut = 0;
		  stabilization = 300;
		  previous = 0;
		  Leader_State = waitTransmit;
		  break;
		  case waitTransmit:
		  //PORTD = 0x20;
		  if (!(USART_IsSendReady(0)))
		  {
			  Leader_State = waitTransmit;
			  break;
		  }
		  if (USART_IsSendReady(0))
		  {
			  Leader_State = transmit;
			  if (occupancyCount>0)
			  {
				  ucTempOut = 0x09;
				  if (isHot)
				  {
					  previous = isHot;
					  for (i=0;i<stabilization;i++)
					  {
						if (!(isHot==previous))
						{
						  	 USART_Send(ucTempOut,0);
						  	 break;	
						}
						  
					  }
					  ucTempOut = 0x03;
				  }
				  
				  USART_Send(ucTempOut,0);
				  break;
			  }
			  else
			  {
				  
				  ucTempOut = 0x0C;
				  USART_Send(ucTempOut,0);
				  break;
			  }

		  }
		  
		  case transmit:
		  //PORTD = 0x40;
		  if (!(USART_HasTransmitted(0)))
		  {
			  Leader_State = transmit;
			  break;
		  }
		  else
		  Leader_State = waitTransmit;
		  break;
		  
		  default:
		  PORTD = 0xFF;
		  break;
	  }
  }

  void LeaderSecTask_Leader()
  {
	  Leader_Init();
	  for(;;)
	  {
		  Leader_State_Tick();
		  vTaskDelay(100);
	  }
  }

  void StartSecPulse_Leader(unsigned portBASE_TYPE Priority)
  {
	  xTaskCreate(LeaderSecTask_Leader, (signed portCHAR *)"LeaderSecTask_Leader", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
  }

enum FolloweState {f_INIT,f_waitRecieve} Follower_State;
void Follower_Init(){
	Follower_State = INIT;
}

void Follower_State_Tick(){
	//Actions
	unsigned char ucTempOut;
	switch(Follower_State){
		case f_INIT:
		break;
		case f_waitRecieve:
		PORTA = USART_Receive(0);
		break;
		
		default:
		break;
	}
	//Transitions
	switch(Follower_State){
		case f_INIT:
		initUSART(0);
		ucTempOut = 0;
		Follower_State = f_waitRecieve;
		break;
		case f_waitRecieve:
		
		Follower_State = f_waitRecieve;
		break;
	}
}

void FollowerSecTask_Follower()
{
	Follower_Init();
	for(;;)
	{
		Follower_State_Tick();
		vTaskDelay(200);
	}
}

void StartSecPulse_Follower(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(FollowerSecTask_Follower, (signed portCHAR *)"FollowerSecTask_Follower", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

enum Light_State {l_INIT,l_wait,l_check,l_pause} l_state;
void l_Init(){
	l_state = l_INIT;
}

void l_Tick(){
	//Actions
	static unsigned char i;
	static unsigned char tempOccupancy;
	switch(l_state){
		case l_INIT:
		break;
		case l_wait:
		tempOccupancy = occupancyCount;
		break;
		case l_check:
		if ((!(tempOccupancy==occupancyCount))&& (isDark))
		{
			light_on = 1;
			break;
		}
		break;
		case l_pause:
		break;
		default:
		break;
	}
	//Transitions
	switch(l_state){
		case l_INIT:
		i = 0;
		tempOccupancy = 0;
		light_on = 0;
		l_state = l_wait;
		break;
		case l_wait:
		if (validateDEC||validateINC)
		{
			l_state = l_check;
			break;
		}
		else{
			l_state = l_wait;
			break;
		}
		
		case l_check:
		if ((i<20) && (light_on == 0))
		{
			i++;
			l_state = l_check;
			break;
		}
		if ((i==20)&&(light_on==0))
		{
			l_state = l_wait;
			i = 0;
			tempOccupancy = occupancyCount;
		break;}
		else{
			l_state = l_pause;
			i =0;
			break;
		}
		case l_pause:
		if (i<75)
		{
			i++;
			l_state = l_pause;
			break;
		}
		else{
			l_state = l_wait;
			i=0;
			tempOccupancy = occupancyCount;
			light_on = 0;
			break;
		}
		default:
		break;
	}
}

void l_Task()
{
	l_Init();
	for(;;)
	{
		l_Tick();
		vTaskDelay(200);
	}
}

void L_Pulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(l_Task, (signed portCHAR *)"l_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}
// This dimmer machine never performed correctly and was left out
/*
enum Dimmer_State {d_INIT,d_wait,d_high,d_low} d_state;
void d_Init(){
	d_state = d_INIT;
}

void d_Tick(){
	//Actions
	static unsigned char i;
	static unsigned char low;
	static unsigned char high;
	static unsigned short j;
	switch(d_state){
		case d_INIT:
		break;
		case d_wait:
		break;
		case d_high:
		dimmer = 1;
		break;
		case d_low:
		dimmer = 0;
		break;
		default:
		break;
	}
	//Transitions
	switch(d_state){
		case d_INIT:
		i = 0;
		low = 1;
		high = 2;
		dimmer = 0;
		j =0;
		d_state =d_wait;
		break;
		case d_wait:
		if (light_on)
		{
			d_state = d_high;
			break;
		}
		else{
			d_state = d_wait;
			break;
		}
		case d_high:
		if (i<high)
		{
			i++;
			d_state = d_high;
			break;
		}
		else{
			d_state = d_low;
			i=0;
		}
		break;
		case d_low:
		if (i<low)
		{
			i++;
			d_state = d_low;
			break;
		}
		else {
			d_state = d_high;
			i = 0;
			if (j<1000)
			{
				j++;
				break;
			}
			else if (low>0)
			{
				high++;
				low--;
				break;
			}
				else
				break;
		}
		
		
		default:
		break;
	}
}

void d_Task()
{
	d_Init();
	for(;;)
	{
		d_Tick();
		vTaskDelay(1);
	}
}

void D_Pulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(d_Task, (signed portCHAR *)"d_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}
*/
enum ADCState {INITADC, setADC, readADC, waitADC} ADC_state;

void ADC_Init(){
	ADC_state = INITADC;
}

void ADC_Tick(){
	static unsigned char i = 7;
	static float voltage;
	//Actions
	switch(ADC_state){
		case INITADC:
		break;
		case setADC:
		Set_A2D_Pin(i);
		break;
		case waitADC:
		break;
		case readADC:
		switch(i){
			case 7:
				voltage = (ADC*4.99)/1024;
				voltage =(voltage-.5)*100;
			i=6;
			if ( voltage > 0.1)
			{
				isHot = 1;
			}
			else isHot = 0;
			break;
			case 6:
			if ( ADC>0x0370)
			{
				isDark = 0;
			}
			else isDark = 1;
			i=7;
			break;
		}
		break;
	}
	
	//Transitions
	switch(ADC_state){
		case INITADC:
		ADC_state =setADC;
		break;
		case  setADC:
		ADC_state = waitADC;
		break;
		case  waitADC:
		ADC_state = readADC;
		break;
		case readADC:
		ADC_state =setADC;
		break;
	}
}

void ADCTask()
{
	ADC_Init();
	for(;;)
	{
		ADC_Tick();
		vTaskDelay(10);
	}
}

void ADCPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(ADCTask, (signed portCHAR *)"ADCTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

//HERE

enum BOOTState {boot_INIT,boot_count, boot_wait} boot_State;
void boot_Init(){
	boot_State = INIT;
}

void boot_Tick(){
	//Actions
	static unsigned char i;
	switch(boot_State){
		case boot_INIT:
		break;
		case boot_count:
		break;
		case boot_wait:
		break;
		default:
		break;
	}
	//Transitions
	switch(boot_State){
		case boot_INIT:
		i=0;
		ON = 0;
		boot_State = boot_count;
		break;
		case boot_count:
		if (i<60)
		{
			i++;
			boot_State = boot_count;
			break;
		}
		else{
			boot_State = boot_wait;
			ON = 1;
		break;}
		case boot_wait:
		boot_State = boot_wait;
		break;
		default:
		break;
	}
}

void boot_Task()
{
	boot_Init();
	for(;;)
	{
		boot_Tick();
		vTaskDelay(1000);
	}
}

void boot_Pulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(boot_Task, (signed portCHAR *)"boot_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}


//Master stub
//PORTB = PIR input
//PORTA lower 5 = debug/registry/light
//PORTA upper 3 = ADC
//PORTD = UART
int main(void)
{
	//float tempC;
	//float voltage;
	A2D_init();
	
	DDRB = 0x00; PORTB = 0xFF;
	//DDRA = 0xFF; PORTA = 0x00;
	DDRD = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	ENTER_Pulse(2);
	EXIT_Pulse(1);
	OUT_Pulse(3);
	OCI_Pulse(4);
	StartSecPulse_Leader(5);
	L_Pulse(6);
	ADCPulse(7);
	boot_Pulse(8);
	// D_Pulse(10); could not get to work
	//StartSecPulse_Follower(4);
	//RunSchedular
	vTaskStartScheduler();
	
	
	//ADC TESTING
	/*
	DDRC = 0xFF;
	DDRD = 0xFF;
	A2D_init();
	Set_A2D_Pin(6);
	while (1)
	{
	//voltage = (ADC*4.99)/1024;
	//voltage =(voltage-.5)*100;
	//tempC = voltage;
		//if (voltage>8.2)
		if(ADC<0x0398){
			PORTD = 0xFF;
			PORTC =  0x03;
	}
	else{
		PORTD = 0;
		PORTC = 0;
	}
  }*/
}

/*
//Slave stub
int main(void)
{
	DDRB = 0x00; PORTB = 0xFF;
	DDRA = 0xFF; PORTA = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	
	StartSecPulse_Follower(4);
    //RunSchedular
	vTaskStartScheduler();
	
}*/
	
