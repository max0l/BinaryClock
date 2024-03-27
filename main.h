#pragma once
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
#include <avr/power.h>

/////////////////////////////////////////////
// Initialise Hour and Minute counter
extern volatile uint8_t hour;
extern volatile uint8_t minute;
extern volatile uint16_t second;
/////////////////////////////////////////////
//Buttons

const uint8_t buttons; 
//Buttons Entprellen

void checkButtons();

volatile uint8_t prell;
volatile uint8_t brightnessLevel; //Helligekeitsstufen
/////////////////////////////////////////////
//LEDS

typedef struct{
	volatile uint8_t* port;
	uint8_t pin;
} Pin;

extern const Pin minLedPins[];

const size_t numMinLedPins;

extern const Pin hourLedPins[];

const size_t numHourLedPins;

// Method Declaration
void displayTime(uint8_t hour, uint8_t minute);
void setEverythingOff();

//sleep modi bool
#define SLEEPDELAY 30
extern volatile bool sleepEnabled;
volatile uint8_t sleepDownTimer;

//Enum State
enum State {
    DISPLAY_TIME,
    SET_HOUR,
    SET_MINUTE,
    ADJUST_BRIGHTNESS, 
    SLEEP_MODE
};

enum State currentState;

void adjustBrightnes(int8_t value);
void sleepButton();
void alterMinute(int8_t value);
void alterHour(int8_t value);
void custom_delay(uint8_t level);

/////////////////////////////////
//negate drift: Every 7 Hours add 1 sekond. Since 2 Sekonds is real: 1,9999220 s --> 0,000078s
//overall drift per our: 0,1404s --> every 7 Hours, add 1 second
#define NEGATEDRIFT 7
volatile uint8_t negateCounter;
