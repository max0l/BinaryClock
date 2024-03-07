#define __AVR_ATmega48a__

#define F_CPU 1000000UL
#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>

/////////////////////////////////////////////
// Initialise Hour and Minute counter
volatile uint8_t hour = 0;
volatile uint8_t minute = 0;

volatile uint16_t ms = 0;
/////////////////////////////////////////////
//Buttons

const uint8_t buttons = (1 << PD0) | (1 << PD1) | (1 << PD2); 
//Buttons Entprellen

volatile uint8_t prell = 0;
/////////////////////////////////////////////
//LEDS
typedef struct{
	volatile uint8_t* port;
	uint8_t pin;
} Pin;
const Pin minLedPins[] = {
	{&PORTB, PB2},
	{&PORTB, PB1},
	{&PORTC, PC0},
	{&PORTC, PC1},
	{&PORTC, PC2},
	{&PORTC, PC3}
		
};

const size_t numMinLedPins = sizeof(minLedPins) / sizeof(minLedPins[0]);

const Pin hourLedPins[] = {
	{&PORTB, PB0},
	{&PORTD, PD7},
	{&PORTD, PD6},
	{&PORTC, PC5},
	{&PORTC, PC4}
	
};

const size_t numHourLedPins = sizeof(hourLedPins) / sizeof(hourLedPins[0]);
/////////////////////////////////
int main() {
	// Setze alle Pins von Port C als Ausgänge
	DDRC = 0b00111111;
	DDRD = 0b11000000;
	DDRB = 0b00000111;
	
	//////////////////////////////////////////////
	//Buttons
	// Aktiviere Pull-Up-Widerstände für die Taster
	PORTD |= buttons;
	
	//Trigger bei IO Chnage bei INT0
	EICRA |= (1<<ISC01) | (1<<ISC00);
	
	//Enable Button1 Interrupt
	EIMSK |= (1<<INT0);
	
	////////////////////////////////////////
	//Clock
	
	//Enable PRT
	//PRR &= (0<<PRTIM2);
	
	//Set AS2 to 1 so TSK1 and TASK2 (external quartz clock)
	ASSR |= (1<<AS2);
	
	//Timer Interrupts
	TCCR2B |= (1<<CS02) | (1<<CS20); //this is the prescaler, needs to be set to 128
	OCR2A = 256-1; // this is the value that the timer counts to
	TCCR2A |= (1<<WGM01); //enable CTC -> Timer wird zurückgesetzt wenn OCR0A erreicht wird
	TIMSK2 |= (1<<OCIE2A); //enable compare Interrupt 1A (of OCR0A)
	

	
	
	sei();

		

	while (1)
	{	
		asm("nop");		
	}
}


//Button 1 Interrupt
ISR(INT0_vect){
	minute = 0;
	hour=0;
	if(prell == 0){
		prell = 10;
	} else {
		return;
	}
	minute = 0;
	hour=0;
	

}

//Timer0 Interrupt
ISR(TIMER2_COMPA_vect) {
	PORTB |= (1<<PB0);
	_delay_ms(1);
	PORTB &= ~(1<<PB0);
}


/*
#include <avr/io.h>

// Reihenfolge der Pins: PIND1, PIND4, PIND3, PIND0, PIND2, PIND5
void setPins(uint8_t mask)
{
	// Erstelle ein Array, das die Reihenfolge der Pins widerspiegelt
	uint8_t pinOrder[] = {PIND1, PIND4, PIND3, PIND0, PIND2, PIND5};
	
	// Gehe durch die Pins in der gew�nschten Reihenfolge
	for (int i = 0; i < 6; i++)
	{
		if (mask & (1 << i)) // �berpr�fe, ob das entsprechende Bit in der Maske gesetzt ist
		{
			// Setze den Pin auf HIGH
			PORTD |= (1 << pinOrder[i]);
		}
		else
		{
			// Setze den Pin auf LOW
			PORTD &= ~(1 << pinOrder[i]);
		}
	}
}

*/