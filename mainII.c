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


enum State {
    DISPLAY_TIME, //muss noch gemacht werden, aber ist im grunde genommen default
    SET_HOUR,
    SET_MINUTE,
    ADJUST_BRIGHTNESS, 
    SLEEP_MODE //könnte man drüber nachdenken ob man das braucht
};

volatile uint8_t hour = 23;
volatile uint8_t minute = 59;
								//	0  1    2   3
volatile uint8_t brightnesses[] = {10, 15, 20, 30}; // werte können noch verändert werden
volatile uint8_t brightnessLevel = 0; //darf nicht größer als 3 werden

enum State currentState = DISPLAY_TIME;

typedef struct {
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

ISR(PCINT2_vect) {
    _delay_ms(50); 
    //zeit setzen
    if ((PIND & (1 << PD0))) {

    	switch(currentState) {
			case DISPLAY_TIME:
			    currentState = SET_HOUR;
				break;
			case SET_HOUR:
				//Ok button -> weiter zur Minute
				currentState = SET_MINUTE;
				break;
			case SET_MINUTE:
				currentState = DISPLAY_TIME;
				break;
			case ADJUST_BRIGHTNESS:
				currentState = DISPLAY_TIME;
				break;
			default:
				currentState = SET_HOUR;
				break;
		}
    
    
		
    }
    if ((PIND & (1 << PD1))) {
	//Helligkeit
	
	   switch(currentState) {
			case DISPLAY_TIME:
			    currentState = ADJUST_BRIGHTNESS;
				break;
			case SET_HOUR:
				//Ok button -> weiter zur Minute
				currentState = SET_MINUTE;
				break;
			case SET_MINUTE:
				currentState = DISPLAY_TIME;
				break;
			case ADJUST_BRIGHTNESS:
				adjustBrightnes(1); //+1
				break;
			default:
				currentState = SET_HOUR;
				break;
		}
	//if state = setHour/setMinute -> ++

    }
    //Kann in ISR(INT0_vect) ausgelagert werden
    if ((PIND & (1 << PD2))) {
		//gehe in den Sleep modus/wache auf
		
		switch(currentState) {
			case DISPLAY_TIME:
				//braucht nicht viel logik, da nur an/aus
				sleepButton();
				break;
			case SET_HOUR:
				//if state = setHour/setMinute -> --
				alterHour(-1);
				break;
			case SET_MINUTE:
				alterMinute(-1);
				break;
			case ADJUST_BRIGHTNESS:
				adjustBrightnes(-1); //+1
				break;
			default:
				sleepButton();
				break;
		}
		
    }
}

// das gleiche mit stunden, bloß mit angepassten werten
void alterMinute(int8_t value) {
	if(value<0 && minute == 0){
		minute = 59;
		return;
	}
	if(value < 0) {
		minute++;
		return;
	}
	if(value > 0 && minute == 59) {
		minute = 0;
		return;
	}
	if(value > 0 ) {
		minute++;
		return;
	}
}

void adjustBrightnes(int8_t value) {
	//genauso wie bei alterMinute...
}

//void sleepButton() {
//	if(sleep) {
//		sleep = false;
//	} else {
//		sleep = true;
//	}
//	sleepDownTimer = SLEEPDELAY;

//}


//hiermit schalten wir die Lampen nacheinander an (~15 us warten) und dann wieder aus, was unsere PWM ist
void displayTime(uint8_t hour, uint8_t minute){
	//das gibt an wie lange die lampen aus sind
	_delay_ms(18); //-> könnte auch noch verändert werden
	for (size_t i = 0; i < minLedPins; i++)
	{
		if (minute & (1 << i))
		{
			*minLedPins[i].port |= (1 << minLedPins[i].pin);
			//das gibt an wie lange die lampen an sind
			_delay_us(brightnesses[brightnessLevel]);
			*minLedPins[i].port &= ~(1 << minLedPins[i].pin);
		}
	}
	
	for (size_t i = 0; i < hourLedPins; i++)
	{
		
		if (hour & (1 << i))
		{
			*hourLedPins[i].port |= (1 << hourLedPins[i].pin);
			//das gibt an wie lange die lampen an sind
			_delay_us(brightnesses[brightnessLevel]+3);
			*hourLedPins[i].port &= ~(1 << hourLedPins[i].pin);
		}
	}
}
