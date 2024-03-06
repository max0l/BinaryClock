/*
 * main.c
 *
 * Created: 11/23/2023 10:37:14 AM
 *  Author: maxfe
 */ 
#define F_CPU 1000000UL

#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

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
int main(void)
{
	// Setze alle Pins von Port C als Ausg�nge
	DDRC = 0b00111111;
	DDRD = 0b11000000;
	DDRB = 0b00000111;
	
	//////////////////////////////////////////////
	//Buttons
	// Aktiviere Pull-Up-Widerst�nde f�r die Taster
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
	ASSR |= 0b00100000;
	
	//Timer Interrupts
	TCCR0B |= (1<<CS02); //ps = 256
	OCR0A=128-1;
	TCCR0A |= (1<<WGM01);
	TIMSK0 |= (1<<OCIE0A);
	

	
	
	sei();

		

	while (1)
	{	
		_delay_us(16);
		PORTC = 0b00000000;
		PORTD &= buttons; // |= 0b00000111;
		PORTB = 0;
		_delay_ms(16);
		if(prell) {
			prell--;
		}
		//Set Minute
		for (size_t i = 0; i < numMinLedPins; ++i) {
			if (minute & (1 << i)) {
				*(minLedPins[i].port) |= (1 << minLedPins[i].pin);
				} else {
				*(minLedPins[i].port) &= ~(1 << minLedPins[i].pin);
			}
		}
		//Set Hour
		for (size_t i = 0; i < numHourLedPins; ++i) {
			if (hour & (1 << i)) {
				*(hourLedPins[i].port) |= (1 << hourLedPins[i].pin);
				} else {
				*(hourLedPins[i].port) &= ~(1 << hourLedPins[i].pin);
			}
		}
		
		
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
ISR(TIMER0_COMPA_vect) {

	minute++;
	if(minute>=60) {
		minute=0;
		hour++;
		if(hour>=24) {
			hour=0;
		}
	}

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