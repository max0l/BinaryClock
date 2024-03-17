#include "main.h"
#include "dcf77.h"

volatile uint8_t hour = 23;
volatile uint8_t minute = 59;

volatile uint16_t second = 0;

volatile uint8_t prell = 0;
volatile uint8_t brightnessLevel = 0;

const uint8_t buttons = (1 << PD0) | (1 << PD1) | (1 << PD2); 

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

const size_t numMinLedPins = sizeof(minLedPins) / sizeof(minLedPins[0]);
const size_t numHourLedPins = sizeof(hourLedPins) / sizeof(hourLedPins[0]);

volatile bool sleep = false;
volatile bool sleepEnabled = true;
volatile uint8_t sleepDownTimer = SLEEPDELAY;

int main() {
	// Setze alle Pins von Port C als Ausgänge
	DDRC = 0b00111111;
	DDRD = 0b11100000;
	DDRB = 0b00000111;
	
	//////////////////////////////////////////////
	//Buttons
	// Aktiviere Pull-Up-Widerstände für die Taster
	PORTD |= buttons;
	
	//Trigger bei IO Chnage bei INT0
	EICRA |= (1<<ISC01) | (1<<ISC00); //+Taster 2
	
	//Enable Button1 Interrupt
	EIMSK |= (1<<INT0);
	
	////////////////////////////////////////
	//Clock
	
	//Enable PRT
	//PRR &= (0<<PRTIM2);

	initDCF77();
	
	//Set AS2 to 1 so TSK1 and TASK2 (external quartz clock)
	ASSR |= (1<<AS2);
	
	//Timer Interrupts
	TCCR2B |= (1<<CS02) | (1<<CS20); //this is the prescaler, needs to be set to 128
	OCR2A = 256-1; // this is the value that the timer counts to
	TCCR2A |= (1<<WGM01); //enable CTC -> Timer wird zurückgesetzt wenn OCR0A erreicht wird
	TIMSK2 |= (1<<OCIE2A); //enable compare Interrupt 1A (of OCR0A)
	

	//Set Sleep mode
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	
	sei();

		

	while (1)
	{	//Maybe the check could be improved somehow
		if(sleep && sleepEnabled) {
			sleep_mode();
		} else {
			displayTime(hour, minute);
		}
		

	}
}


//Button 1 Interrupt
ISR(INT0_vect){
	//Entprellen
	/*
	second = 0;
	minute = 0;
	hour=0;
	*/
	//switch sleep
	if(sleep) {
		sleep = false;
	} else {
		sleep = true;
	}
	sleepDownTimer = SLEEPDELAY;

}
//Button 2
/*
ISR(INT1_vect) { // Neue ISR für Taster 2 (Stunden und Minuten einstellen)
    static bool settingHours = true; // Zustand zwischen Stunden und Minuten umschalten
    if(settingHours) {
        hour = (hour + 1) % 24;
    } else {
        minute = (minute + 1) % 60;
    }
    settingHours = !settingHours;
}
*/
/* Das geht nicht, da es kein INT2 gibt. Du musst das über den Pinchange Interrupt machen
//Button 3
ISR(INT2_vect) { // Neue ISR für Taster 3 (Helligkeit anpassen)
    brightnessLevel = (brightnessLevel + 1) % 4; // 4 Helligkeitsstufen
}
*/
//Timer0 Interrupt
ISR(TIMER2_COMPA_vect) {
	second++;
	if(second == 60){
		second = 0;
		minute++;
		if(minute == 60){
			minute = 0;
			hour++;
			if(hour == 24){
				hour = 0;
			}
		}
	}
	//if portd4 is on
	if(PIND & (1<<PD5)){
		PORTD &= ~(1 << PD5);
	} else {
		PORTD |= (1 << PD5);
	}

	if(!sleep && sleepEnabled){
		sleepDownTimer--;
		if(sleepDownTimer == 0){
			sleepDownTimer = SLEEPDELAY;
			sleep = true;
			
		}
	}
	

}

//Method to let the LEDs display the time
//this could be more power efficient if each LED is turned on and off individually (and in order)
void displayTime(uint8_t hour, uint8_t minute){
	_delay_ms(18);
	for (size_t i = 0; i < numMinLedPins; i++)
	{
		if (minute & (1 << i))
		{
			*minLedPins[i].port |= (1 << minLedPins[i].pin);
			_delay_us(15);
			*minLedPins[i].port &= ~(1 << minLedPins[i].pin);
		}
	}
	
	for (size_t i = 0; i < numHourLedPins; i++)
	{
		
		if (hour & (1 << i))
		{
			*hourLedPins[i].port |= (1 << hourLedPins[i].pin);
			_delay_us(18);
			*hourLedPins[i].port &= ~(1 << hourLedPins[i].pin);
		}
	}
}
