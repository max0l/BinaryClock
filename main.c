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
void setPins(uint8_t value);
/////////////////////////////////////////////////////////////////////////////////
// 2 Möglichkeiten:

// Definieren Sie eine Struktur, um Port- und Pin-Informationen zu speichern
typedef struct{
	volatile uint8_t* port;
	uint8_t pin;
} Pin;
// Deklarieren Sie Ihre Pins
const Pin pins[] = {
	{&PORTC, PD7},  // Beispiel: PORTD, Pin D1
	{&PORTC, PD6},  // Beispiel: PORTD, Pin D3
	{&PORTC, PD5},  // Beispiel: PORTD, Pin D0
	{&PORTC, PC4},   // Beispiel: PORTC, Pin C4
	{&PORTC, PD3},
	{&PORTC, PC2},
	{&PORTC, PC1},
	{&PORTC, PD0}
		
};

const size_t numPins = sizeof(pins);


// Oder so:---------------------------------------------------------------

const uint8_t minPortOrder[] = {&PORTD, &PORTD, &PORTD, &PORTD, &PORTD, &PORTD};
const uint8_t minPinOrder[] = {PIND1, PIND4, PIND3, PIND0, PIND2, PIND5};
const uint8_t hourPortOrder[] = {PIND1, PIND4, PIND3, PIND0, PIND2, PIND5};
const uint8_t hourPinOrder[] = {PIND1, PIND4, PIND3, PIND0, PIND2, PIND5};
////////////////////////////////////////////////////////////////////////////

int main(void)
{
	//Set AS2 to 1 so TSK1 and TASK2 (external quartz clock)
	//ASSR |= 0b00100000;
	
	// Setze alle Pins von Port C als Ausgänge
	DDRC = 0xFF;
	
	// Aktiviere Pull-Up-Widerstände für die Taster
	PORTD |= (1 << PD0) | (1 << PD1) | (1 << PD2); 

	uint8_t counter = 0;
	while (1)
	{
		if (!(PIND & (1 << PIND2))) // Überprüfe den Zustand des Tasters an PD1
		{
			_delay_ms(200);
			counter++;
			if (counter == 64)
			{
				counter = 0;
			}

		}

		//setLEDs(0b00111111);
		setPins(counter);
		
	}
}

void setLEDs(uint8_t i)
{
	PORTC = i;
	_delay_us(90);
	PORTC = 0b00000000;
	_delay_ms(5);
}

void setPins(uint8_t value) {
	for (size_t i = 0; i < numPins; ++i) {
		if (value & (1 << i)) {
			*(pins[i].port) |= (1 << pins[i].pin);  // Pin setzen
			} else {
			*(pins[i].port) &= ~(1 << pins[i].pin); // Pin löschen
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