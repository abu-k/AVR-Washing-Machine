#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t finish_flag = 0;
volatile uint8_t running_flag = 0;
volatile uint16_t count = 0;
volatile uint8_t led_index = 1;

/*Function to display data onto SSD*/
void display_SSD(uint8_t side)
{
	uint8_t characer;
	
	uint8_t low = (1 << 3);
	uint8_t med = (1 << 6);
	uint8_t high = (1 << 0);
	uint8_t norm = (1 << 2) | (1 << 4) | (1 << 6);
	uint8_t letter_e = (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
	uint8_t zero = (1 << 0) |  (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
	/* Output the current digit */
	if (finish_flag)
	{
		PORTA = (side << PORTA7) | (zero & 0b01111111);
	}
	else
	{
		if(side == 0) {
			if ((PIND & 0b11) == 0)
			{
				characer = low;
			}
			else if ((PIND & 0b11) == 2)
			{
				characer = med;
			}
			else if ((PIND & 0b11) == 1)
			{
				characer = high;
			}
			else
			{
				characer = letter_e;
			}
			} else {

			if ((PIND & (1 << 4)) == 0)
			{
				characer = norm;
			}
			else
			{
				characer = letter_e;
			}
		}
		PORTA = (side << PORTA7) | (characer & 0b01111111);
	}
	
	
}

/*Function to display regular running leds from left to right for 3 seconds*/
void running_leds_left_to_right()
{
	OCR1A = 31250; /* Clock divided by 8 - count for 10000 cycles */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10); 
	TIMSK1 = (1<<OCIE1A); 

	TIFR1 = (1<<OCF1A);
	sei();
	count = 0;
	while(count < 16){
		PORTB = (1 << count%4);
	}
	PORTB = 0;
}

/*Function to display running leds in both directions for 3 seconds*/
void running_leds_both()
{
	uint8_t direction= 0;
	OCR1A = 31250; /* Clock divided by 8 - count for 10000 cycles */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10);
	TIMSK1 = (1<<OCIE1A);

	TIFR1 = (1<<OCF1A);
	sei();
	count = 0;
	while(count < 16){
		if (count > 0 && count <= 3)
		{
			PORTB = (1 << ((count%4)+1));
		}
		else if (count >= 4 && count <= 7)
		{
			PORTB = (1 << (4-(count%4)));
		}
		else if (count >= 8 && count <=11)
		{
			PORTB = (1 << ((count%4)+1));
		}
		else if (count >= 12 && count <= 15)
		{
			PORTB = (1 << (4-(count%4)));
		}
	}
	PORTB = 0;
}

/*Function to display regular solid leds for 3 seconds*/
void solid_leds(){
	OCR1A = 31250; /* Clock divided by 8 - count for 10000 cycles */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10);
	TIMSK1 = (1<<OCIE1A);

	TIFR1 = (1<<OCF1A);
	sei();
	count = 0;
	while(count < 16){
		PORTB = 0b1111;
	}
	PORTB = 0;
}

/*Function to display all 4 leds flashing*/
void flashing_leds(){
	OCR1A = 31250; /* Clock divided by 8 - count for 10000 cycles */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10);
	TIMSK1 = (1<<OCIE1A);

	TIFR1 = (1<<OCF1A);
	sei();
	count = 0;
	while(count < 16){
		if ((count % 2) == 1)
		{
			PORTB = 0b1111;
		} 
		else
		{
			PORTB = 0;
		}
	}
	PORTB = 0;
}

/*Function to display all 4 leds flashing at double the rate of flashing_leds()*/
void flashing_leds_double_time(){
	OCR1A = 15625; /* Clock divided by 8 - count for 10000 cycles */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10);
	TIMSK1 = (1<<OCIE1A);

	TIFR1 = (1<<OCF1A);
	sei();
	count = 0;
	while(count < 32){
		if ((count % 2) == 1)
		{
			PORTB = 0b1111;
		}
		else
		{
			PORTB = 0;
		}
	}
	PORTB = 0;
}

/*Washing machine wash cycle*/
void wash_cycle()
{
	OCR0B = 229;
	running_leds_left_to_right();
	solid_leds();
}

/*Washing machine rinse cycle*/
void rinse_cycle()
{
	OCR0B = 127;
	running_leds_left_to_right();
	flashing_leds();
}

/*Washing machine spin cycle*/
void spin_cycle()
{	
	OCR0B = 25;
	running_leds_both();
	flashing_leds_double_time();
}

/*Washing machine finish cycle*/
void finish_cycle()
{
	OCR0B = 255;
	finish_flag = 1;

}

/*
 * main -- Main program
 */
int main(void)
{
	uint8_t side; /* 0 = right, 1 = left */
	
	/* Ensure interrupt flag is cleared */
	TIFR1 = (1 << OCF1A); 
	
	/* Set Timer/Counter Control Registers*/
	TCCR0A = (1 << COM0B1) | (1 << COM0B0) | (1 << WGM01) | (1 << WGM00);
	TCCR0B = (1 << CS00);
	
	/* Set up interrupt to occur on falling edge of pin D2 (start/stop button) */
	EICRA = (1 << ISC01) | (0 << ISC00); /* Falling edge */
	EIMSK = (1 << INT0);
	EIFR = (1 << INTF0);
	
	/* Set up interrupt to occur on rising edge of pin D3 (reset button) */
	/* Note use of |= for the first two registers so as to not overwrite the
	** values we set above.
	*/
	EICRA |= (1 << ISC11) | (0 << ISC10);  /* Falling edge */
	EIMSK |= (1 << INT1);
	EIFR = (1 << INTF1);

	/* Turn on global interrupts */
	sei(); 

	/* Set port A (all pins) to be outputs for SSD */
	DDRA = 0xFF;
	/* Set 5 LSB on port B to be outputs for LEDs */
	DDRB = (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
    /* Set 5 LSB on port D to be inputs from buttons and switches*/
    DDRD = (0 << 0) | (0 << 2) | (0 << 3) | (0 << 4) | (0 << 5);
	
	/* LED7 off*/
	OCR0B = 255;
	
	side = 0;
	sei();
	while(1) {
		
		side = 1 - side;
		display_SSD(side);
		
	}
}
/* Interrupt handler for event on pin D2 (external interrupt 0 - Button 0).*/
ISR(INT0_vect) {
	/*Normal Mode*/
	
	if (running_flag==0)
	{
		running_flag =1;
	
	if (((PIND & 0b11) != 3))
	{
		if ((PIND & (1 << 4)) == 0)
		{
			wash_cycle();
			rinse_cycle();
			spin_cycle();
			finish_cycle();
			
		}
		/*Extended Mode*/
		else
		{
			wash_cycle();
			rinse_cycle();
			rinse_cycle();
			spin_cycle();
			finish_cycle();
		}
	}
	}
	
}

/* Interrupt handler for event on pin D3 (external interrupt 1 - Button 1).*/
ISR(INT1_vect) {
	finish_flag = 0;
	running_flag = 0;
}

/* Interrupt handler for event triggered timer 1*/
ISR(TIMER1_COMPA_vect){
	count+=1 ;
}