#include <avr\io.h>

#include <avr\interrupt.h>

#include <ctype.h>
#include <avr\iom64.h>
#define ICP (PIND & 0x10)
volatile unsigned char first;
//define ovrflow counter

uint16_t ov_counter;

//define times for start and end of signal

uint16_t rising, falling;

//define overall counts

uint16_t counts;

//overflow counter interrupts service routine


void init_usart(void)
{
	UCSR1B = 0x98;	// tx-polling	rx-interrupt 
	UCSR1C = 0x06;	//8N1
	UBRR1L=11;		//38400
	UBRR1H=0;
}

void Usart1_Tx(unsigned char data)			
{

	UDR1 = data;
	while (!(UCSR1A & 0x40));
	UCSR1A |= 0x40;
}

void Usart1_Str(unsigned char *str)
{
	while(*str)
		Usart1_Tx(*str++);
}

ISR(TIMER1_OVF_vect)
{

  ov_counter++;
	UDR1 = 'O';
}

//Timer1 capture interrupt service subroutine

ISR(TIMER1_CAPT_vect)
{

/*This subroutine checks was it start of pulse (rising edge)

or was it end (fallingedge)and performs required operations*/
//UDR1 = 'C';

	if ( ICP ) //if high level

		{

		//save start time

		rising=ICR1;
		//rising=TCNT1;
	//	UDR1 = rising;
		//set to trigger on falling edge

		TCCR1B=TCCR1B&0xBF;

		//reset overflow counter
		

		ov_counter=0;

//		UDR1 = '1';

		}

	else

		{
//		TIMSK=0;
		//save falling time
		
		falling=ICR1;
		//falling=TCNT1;
		//rising edge triggers next

		TCCR1B=TCCR1B|0x40;

		//counts=((falling +(ov_counter*0xFFFF))-rising);
		counts=(falling - rising);

		/*you can convert coutns to seconds and send to LCD*/
		//sprintf(sprint_buf,"COUNTS = %u\n",counts);
		TCNT1 = 0;
		first = 0;
	//	Usart1_Tx('\n');
		Usart1_Tx( (unsigned char)((counts&0xff00)>>8) );	// MSB
		Usart1_Tx((unsigned char)(counts&0x00ff));			// LSB
	//	Usart1_Tx('\n');
		TIMSK=0x24;
//		UDR1 = '2';
		}
//		TCNT1 = 0;		// make the count zero while leaving interrupt
}





int main(void) 
{
	first = 0;
	//enable overflow and input capture interrupts
	TIMSK=0x24;

	/*Noise canceller, without prescaler, rising edge*/

	TCCR1B = 0xC1;
//	TCNT1 = 0;
	
	init_usart();
	Usart1_Tx('a');
	
	sei();

    for (;;)
	{

/* loop forever timer does the job*/

    }
}