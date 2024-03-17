#ifndef MAIN_H
#define MAIN_H
#define __AVR_ATmega48A__

#define F_CPU 1000000UL
#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/portpins.h>
#include <avr/sleep.h>
#include <stdbool.h>

/////////////////////////////////////////////
// Initialise Hour and Minute counter
extern volatile uint8_t hour;
extern volatile uint8_t minute;
extern volatile uint16_t second;
/////////////////////////////////////////////
//Buttons

extern const uint8_t buttons; 
//Buttons Entprellen

extern volatile uint8_t prell;
extern volatile uint8_t brightnessLevel; //Helligekeitsstufen
/////////////////////////////////////////////
//LEDS

typedef struct{
	volatile uint8_t* port;
	uint8_t pin;
} Pin;

extern const Pin minLedPins[];

extern const size_t numMinLedPins;

extern const Pin hourLedPins[];

extern const size_t numHourLedPins;

// Method Declaration
void displayTime(uint8_t hour, uint8_t minute);

//sleep modi bool
#define SLEEPDELAY 10
extern volatile bool sleep;
extern volatile bool sleepEnabled;
extern volatile uint8_t sleepDownTimer;

//Power save
//There are other registers that could be set to save more power


/////////////////////////////////
#endif