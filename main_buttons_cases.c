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

typedef enum {
    DISPLAY_TIME,
    SET_HOUR,
    SET_MINUTE,
    ADJUST_BRIGHTNESS, // Dieser Zustand bleibt ungenutzt in diesem Beispiel
    SLEEP_MODE
} State;

volatile State currentState = DISPLAY_TIME;
volatile uint8_t hour = 23;
volatile uint8_t minute = 59;
volatile uint16_t second = 0;
const uint8_t buttons = (1 << PD0) | (1 << PD1) | (1 << PD2);

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

const Pin hourLedPins[] = {
    {&PORTB, PB0},
    {&PORTD, PD7},
    {&PORTD, PD6},
    {&PORTC, PC5},
    {&PORTC, PC4}
};

#define SLEEPDELAY 100
volatile bool sleep = false;
volatile uint8_t sleepDownTimer = SLEEPDELAY;

void displayTime(uint8_t hour, uint8_t minute); // Diese Funktion muss implementiert werden.
void setEverythingOff();

void buttons_init() {
    DDRD &= ~buttons;
    PORTD |= buttons;
    PCMSK2 |= (1 << PCINT16) | (1 << PCINT17);
    PCICR |= (1 << PCIE2);
}

ISR(PCINT2_vect) {
    _delay_ms(50); // Entprellung
    if (!(PIND & (1 << PD0))) {
        if (currentState == DISPLAY_TIME) currentState = SET_HOUR;
        else if (currentState == SET_HOUR) currentState = SET_MINUTE;
        else if (currentState == SET_MINUTE) currentState = DISPLAY_TIME;
    }
    if (!(PIND & (1 << PD1))) {
        if (currentState == SET_HOUR) hour = (hour + 1) % 24;
        else if (currentState == SET_MINUTE) minute = (minute + 1) % 60;
    }
    if (!(PIND & (1 << PD2))) {
        if (currentState == SET_HOUR) hour = (hour == 0) ? 23 : hour - 1;
        else if (currentState == SET_MINUTE) minute = (minute == 0) ? 59 : minute - 1;
    }
}

int main() {
    buttons_init();
    sei();
    while (1) {
        if (sleep) {
            sleep_mode();
        } else {
            _delay_ms(18); // Kurze Verzögerung
        }
    }
}

ISR(TIMER2_COMPA_vect) {
    second++;
    if (second >= 60) {
        second = 0;
        minute++;
        if (minute >= 60) {
            minute = 0;
            hour++;
            if (hour >= 24) {
                hour = 0;
            }
        
        }
    }
}

void setEverythingOff() {
    // Implementiere die Logik, um alle Ausgänge/LEDs auszuschalten
}