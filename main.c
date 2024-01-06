/*
 * main.c
 *
 * Created: 11/23/2023 10:37:14 AM
 *  Author: maxfe
 */ 
#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

void setLEDs(uint8_t);
void setTime(uint8_t minute, uint8_t hour);

const uint8_t buttons = (1 << PD0) | (1 << PD1) | (1 << PD2); 
/////////////////////////////////////////////////////////////////////////////////
// 2 Möglichkeiten:

// Definieren Sie eine Struktur, um Port- und Pin-Informationen zu speichern
typedef struct{
	volatile uint8_t* port;
	uint8_t pin;
} Pin;
// Deklarieren Sie Ihre Pins
const Pin minLedPins[] = {
	{&PORTB, PB2},
	{&PORTB, PB1},
	{&PORTC, PC0},
	{&PORTC, PC1},
	{&PORTC, PC2},
	{&PORTC, PC3}
		
};

const size_t numMinLedPins = sizeof(minLedPins);

const Pin hourLedPins[] = {
	{&PORTB, PB0},
	{&PORTD, PD7},
	{&PORTD, PD6},
	{&PORTC, PC5},
	{&PORTC, PC4}
	
};

const size_t numHourLedPins = sizeof(hourLedPins);

int main(void)
{
	//Set AS2 to 1 so TSK1 and TASK2 (external quartz clock)
	//ASSR |= 0b00100000;
	
	// Setze alle Pins von Port C als Ausgänge
	DDRC = 0b00111111;
	DDRD = 0b11000000;
	DDRB = 0b00000111;
	
	
	// Aktiviere Pull-Up-Widerstände für die Taster
	PORTD |= buttons;

	uint8_t counter = 0;
	while (1)
	{
		if ((PIND & (1 << PIND2))) // Überprüfe den Zustand des Tasters an PD1
		{
			_delay_ms(200);
			counter++;
			if (counter == 64)
			{
				counter = 0;
			}

		}

		//setLEDs(0b00111111);
		setTime(counter, counter);
		
	}
}

void setLEDs(uint8_t i)
{
	PORTC = i;
	_delay_us(90);
	PORTC = 0b00000000;
	_delay_ms(5);
}

void setTime(uint8_t minute, uint8_t hour) {
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
	_delay_us(380);
	PORTC = 0b00000000;
	PORTD &= buttons; // |= 0b00000111;
	PORTB = 0;
	_delay_ms(550);
}

/*
#include <avr/io.h>

// Reihenfolge der Pins: PIND1, PIND4, PIND3, PIND0, PIND2, PIND5
void setPins(uint8_t mask)
{
	// Erstelle ein Array, das die Reihenfolge der Pins widerspiegelt
	uint8_t pinOrder[] = {PIND1, PIND4, PIND3, PIND0, PIND2, PIND5};
	
	// Gehe durch die Pins in der gewünschten Reihenfolge
	for (int i = 0; i < 6; i++)
	{
		if (mask & (1 << i)) // Überprüfe, ob das entsprechende Bit in der Maske gesetzt ist
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