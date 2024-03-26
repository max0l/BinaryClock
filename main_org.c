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

// Zustände
typedef enum {
    DISPLAY_TIME,
    SET_HOUR,
    SET_MINUTE,
    ADJUST_BRIGHTNESS,
    SLEEP_MODE
} State;

volatile State currentState = DISPLAY_TIME;

volatile uint8_t hour = 23;
volatile uint8_t minute = 59;
volatile uint16_t second = 0;
volatile uint8_t prell = 0;
volatile uint8_t brightnessLevel = 0;
#define SLEEPDELAY 100
volatile bool sleep = false;
volatile uint8_t sleepDownTimer = SLEEPDELAY;

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

const size_t numHourLedPins = sizeof(hourLedPins) / sizeof(hourLedPins[0]);

void displayTime(uint8_t hour, uint8_t minute);
void setEverythingOff();

int main() {
    // Konfiguriere alle Pins als Ausgänge
    DDRC = 0b00111111;
    DDRD = 0b11010000;
    DDRB = 0b00000111;

    // Aktiviere Pull-Up-Widerstände für die Taster
    PCICR |= (1 << PCIE2); // Pin Change Interrupt Control Register
    PCMSK2 |= (1 << PCINT16) | (1 << PCINT17) | (1 << PCINT18); // Pin Change Mask Register 2

    ASSR |= (1 << AS2);
    TCCR2B |= (1 << CS02) | (1 << CS20);
    OCR2A = 256-1;
    TCCR2A |= (1 << WGM01);
    TIMSK2 |= (1 << OCIE2A);

    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sei();

    while (1) {
        switch (currentState) {
            case DISPLAY_TIME:
                displayTime(hour, minute);
                break;
            case SLEEP_MODE:
                sleep_mode();
                break;
            default:
                _delay_ms(18);
                break;
        }
    }
}

ISR(PCINT2_vect) { // Pin Change Interrupt Service Routine für PCINT16, PCINT17, PCINT18
    if (PIND & (1 << PD4)) { // Beispiel für die Überprüfung eines Pins, Anpassung nötig
        switch (currentState) {
            case DISPLAY_TIME:
                // Button Logik hier implementieren
                break;
            case SET_HOUR:
            case SET_MINUTE:
            case ADJUST_BRIGHTNESS:
                // Ändere die Zustände basierend auf dem gedrückten Knopf
                break;
            case SLEEP_MODE:
                // Verlasse den Schlafmodus
                currentState = DISPLAY_TIME;
                break;
        }
    }
}

ISR(TIMER2_COMPA_vect) {
    // Timer Interrupt Logik hier
}

void displayTime(uint8_t hour, uint8_t minute) {
    // Anzeigelogik;
}

void setEverythingOff() {
    // Alles abschalten
}