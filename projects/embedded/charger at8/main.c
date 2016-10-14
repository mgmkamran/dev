/**************************************************************** 
		PROJECT 		:		ON_CHARGER
		CPU				: 		ATmega8
		CPU Speed 		:		7.37 MHZ (External Crystal)
		BAUD RATE		:		38400 bps
		Date			:		03/08/2013
		Last modified	: 		05/08/2013
*******************************************************************/

#include<avr/io.h>
#include<util/delay.h>
#include<string.h>
#include<avr/interrupt.h>
#include<stdlib.h>
#include<stdio.h>


#define	NEWLINE	usart_tx(0x0d);usart_tx(0x0a)


//initialization of ports and usart is not required as it is done in boot loader programm

/**************** USART FUNCTIONS DECLERATIONS ******************/
void usart_tx(char );
void usart_str(char *);
/**************** USART FUNCTIONS DECLERATIONS END ******************/


/**************** ADC FUNCTIONS DECLERATIONS ******************/
void init_recv_condition (void);
void init_adc_condition (void);
void adc_init(void);
/**************** ADC FUNCTIONS DECLERATIONS END******************/



/**************** GLOBAL VARIABLES DECLERATIONS ******************/
volatile char gv_usart_flg,gv_usart[30], gu8v_adc_flg ; 
int i;
volatile int g16v_adc_cnt;
volatile unsigned int gu16v_adc_val;
char tempbuff[30];
/**************** GLOBAL VARIABLES DECLERATIONS ******************/



int main( void )
{
	float vltg;
	usart_str("...........this is a test............");		//
	sei();													// ENABLE ALL INTERRUPTS AS ADC AND USART_RX ARE INITIALISED IN INTERRUPT MODE
	adc_init();												// INITIALISE ADC IN SINGLE RUNNING,INTERRUPT, /64 PRESCALAR MODE
	
	ADCSRA |= (1<<ADSC);									// START THE ADC CONVERSION FOR THE FIRST TIME, WE COULD HAVE STARTED IT WHILE INITIALISING ADC BUT IT WOULD TAKE 25 CPU CYCLES INSTED OF 13.
	
	while(1)
	{
	// SEND THE ENTERD STRING TO SERIAL PORT
		if( gv_usart_flg )
		{
		 usart_str( (char *)gv_usart );
		 
		 init_recv_condition();
		}
	 
		if( gu8v_adc_flg )
		{
		// READING OF 16 VALUES IS OVER 
		 gu16v_adc_val /= 16;									//TAKE THE AVERAGE
//		 utoa(gu16v_adc_val , tempbuff, 10);			
//		 usart_str( tempbuff );
//		 NEWLINE;
		 vltg = ((float) 5.0 * (float) gu16v_adc_val )/((float)1024.00);	// CALCULATE THE ACTUAL VOLTAGE FROM AVERAGED ADC COUNT
		 sprintf(tempbuff,"V = %6.2f\r\n",(double) vltg);					// BRING THE VOLTAGE IN DISPLAYABLE FORMAT
		 usart_str( tempbuff );												// SEND THE VLTG. VALUE TO SERIAL PORT
		 _delay_ms(250);													// GIVE A SMALL DELAY 
//		 NEWLINE;
		 
		 init_adc_condition();									// CLEAR ADC BUFFER, FLAG AND COUNT			
		 // START A NEW SET OF 16 CONVERSIONS
		 ADCSRA |= (1<<ADSC);
		}
	 
	}
//	usart_str("")
return 0;
}


/************ functions related to usart ************/

void usart_tx(char data)
{
    UDR = data;
	while (!(UCSRA & (1<<UDRE)));	// poll for data end flag
	 UCSRA 	|=	(1<<UDRE);			// clear UDRE bit
	
}


void usart_str(char *str)
{
	while(*str)
	 usart_tx(*str++);
//	NEWLINE;
}
 
ISR(USART_RXC_vect)
{
// CHARECTERS ENTERD THROUGH SERIAL PORT WILL BE STORED IN "gv_usart[i]" ARRAY 
	gv_usart[i] = UDR;
	
	if (gv_usart[i] == 0x0d || gv_usart[i] == 0x0a)// is this CR or LF
	{
	 gv_usart[i] 	= '\0';							// close the buffer by NULL terminate the string
	 gv_usart_flg 	= 1;							// indicate to main program that a string has come
	}			
	if (i < 29) i++;	
// AS SOON AS THE ENETR IS HIT FOLLOWING A STRING, THE FLAG WILL ASSERT CAUSING THE STRING TO APPEAR ON TERMINAL
}

void init_recv_condition (void)
{
	i 	= 0;			// BRING THE POINTER OF ARRAY TO FIRST POSSITION....i.e CLEARING THE "gv_usart" BUFFER
	gv_usart_flg = 0;	// CLEAR THE USART_RX FLAG 
}

/************end of functions related to usart ************/



/************   funct ions related to ADC   ***************/

void adc_init(void)
{
	ADCSRA	|= 	0b10001110;			//enable adc,single running,interrupt enable,/64prescalar
	ADMUX	 = 	0b01000001;			//Internal 2.56V as ref, channel1	
	g16v_adc_cnt = 0;
}

ISR (ADC_vect)
{
// AS SOON AS THE ADC CONVERSI0N OVER THIS SUBROUTINE IS CALLED.
// THIS ISR IS PROGRAMMED TO READ 16 VALUES FROM ADC.// IN THE MAIN WE WILL TAKE AVERAGE OF THEM.
//	unsigned char ch;
//	ch = ADCL;			// first read lower byte
//	gu16v_adc_val += ((unsigned int) (ADCH << 8) | (unsigned int) (ch & 0x00ff));		// accumalate ADC readings
	gu16v_adc_val += ADC;		// ADD ADC VALUES IN THE VARIABLE
	g16v_adc_cnt++;				// INCREASE THE COUNT AFTER EACH READ+ADDITION
	
	if(g16v_adc_cnt < 16)		// CHECK FOR 16 VALUES
	 ADCSRA |= (1<<ADSC);		// START ANOTHER CONVERSION IF 16 READINGS ARE NOT OVER.
	else
	 gu8v_adc_flg |= 1;			// INDICATES 16 READINGS ARE DONE. MAKE THE GLOBAL ADC FLAG ONE TO PERFORM DESIRED OPERATION
}

void init_adc_condition (void)
{
// CLEAR ADC BUFFER, FLAG AND COUNT
	gu8v_adc_flg  = 0;			
	gu16v_adc_val = 0;
	g16v_adc_cnt  = 0;
}
/************   funct ions related to ADC END  ***************/