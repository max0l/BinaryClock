#include "main.h"
#include "dcf77.h"

volatile uint8_t hour = 0;
volatile uint8_t minute = 0;

volatile uint16_t second = 0;

const uint8_t buttons = (1 << PD0) | (1 << PD1) | (1 << PD2); 
//Buttons Entprellen

volatile uint8_t prell = 0;
volatile uint8_t brightnessLevel = 0; //Helligekeitsstufen
/////////////////////////////////////////////
//LEDS

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



//sleep modi bool
volatile bool sleepEnabled = true;
volatile uint8_t sleepDownTimer = SLEEPDELAY;


/////////////////////////////////////////////
//State
enum State currentState = DISPLAY_TIME;

//Power save
//There are other registers that could be set to save more power


/////////////////////////////////

int main() {
	// Setze alle Pins von Port C als Ausgänge
	DDRC = 0b00111111;
	DDRD = 0b11110000;
	DDRB = 0b00000111;
	
	//////////////////////////////////////////////
	//Buttons
	// Aktiviere Pull-Down-Widerstände für die Taster
	PORTD |= buttons;
	
	//Trigger bei IO Chnage bei INT0
	//EICRA |= (1<<ISC01) | (1<<ISC00) | (1<<ISC10) | (1<<ISC11); //+Taster 2
	
	//Enable Button1 Interrupt
	//EIMSK |= /*(1<<INT0) |*/ (1<<INT1); // interrupt Taster 2

	//PCINT2
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (1<<PCINT16) | (1<<PCINT17) | (1<<PCINT18);
	
	
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
		if(currentState == SLEEP_MODE && sleepEnabled) {
			sleep_mode();
		} else {
			//_delay_ms(18);
			if(currentState == DISPLAY_TIME) {
				displayTime(hour, minute);
			} else if(currentState == SET_HOUR || currentState == SET_MINUTE) {
				displayTime(~hour, ~minute);
			} else if (currentState == ADJUST_BRIGHTNESS) {
				displayTime(31, 63);
			}
		}
		

	}
}

ISR(PCINT2_vect) {
	//Da die dieser Interrupt bei jeder Flanke ausgelöst wird,
	//Soll der rest erst beim loslassen des Tasters ausgeführt werden
	if(prell) {
		prell--;
		return;
	}
	prell = 1;


    if (!(PIND & (1 << PD0))) {

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
			case SLEEP_MODE:
				sleepButton();
				break;
			default:
				currentState = SET_HOUR;
				break;
		}
    
    
		
    }
    if (!(PIND & (1 << PD1))) {
	   switch(currentState) {
			case DISPLAY_TIME:
			    currentState = ADJUST_BRIGHTNESS;
				break;
			case SET_HOUR:
				alterHour(1);
				break;
			case SET_MINUTE:
				alterMinute(1);
				break;
			case ADJUST_BRIGHTNESS:
				adjustBrightnes(1);
				break;
			case SLEEP_MODE:
				sleepButton();
				break;
			default:
				currentState = ADJUST_BRIGHTNESS;
				break;
		}

    }
    //Kann in ISR(INT0_vect) ausgelagert werden
    if (!(PIND & (1 << PD2))) {
		//gehe in den Sleep modus/wache auf
		
		switch(currentState) {
			case DISPLAY_TIME:
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
			case SLEEP_MODE:
				sleepButton();
				break;
			default:
				sleepButton();
				break;
		}
		
    }
	
}

//Button 1 Interrupt
//ISR(INT0_vect){
//	//Entprellen
//	/*
//	second = 0;
//	minute = 0;
//	hour=0;
//	*/
//	//switch sleep
//	if(sleep) {
//		sleep = false;
//	} else {
//		sleep = true;
//	}
//	sleepDownTimer = SLEEPDELAY;
//
//}

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
	if(PIND & (1<<PD4)){
		PORTD &= ~(1 << PD4);
	} else {
		PORTD |= (1 << PD4);
	}

	if(currentState != SLEEP_MODE && sleepEnabled){
		sleepDownTimer--;
		if(sleepDownTimer == 0){
			sleepDownTimer = SLEEPDELAY;
			currentState = SLEEP_MODE;
			
		}
	}
	

}

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

void alterHour(int8_t value) {
	if(value<0 && hour == 0){
		hour = 23;
		return;
	}
	if(value < 0) {
		hour++;
		return;
	}
	if(value > 0 && hour == 23) {
		hour = 0;
		return;
	}
	if(value > 0) {
		hour++;
		return;
	}
}

void adjustBrightnes(int8_t value) {
	if(value < 0 && brightnessLevel == 0) {
		brightnessLevel = 3;
		return;
	}
	if(value < 0) {
		brightnessLevel--;
		return;
	}
	if(value > 0 && brightnessLevel == 3) {
		brightnessLevel = 0;
		return;
	}
	if(value > 0) {
		brightnessLevel++;
		return;
	}
}

void custom_delay(uint8_t level) {
    switch(level) {
        case 0: _delay_ms(25); break;
        case 1: _delay_ms(15); break;
        case 2: _delay_ms(10); break;
		case 3: _delay_ms(5); break;
		case 4: _delay_ms(2); break;
        default: _delay_ms(25);
    }
}

void sleepButton() {
	if(currentState == SLEEP_MODE) {
		currentState = DISPLAY_TIME;
	} else {
		currentState = SLEEP_MODE;
	}
	sleepDownTimer = SLEEPDELAY;

}

//Method to let the LEDs display the time
//this could be more power efficient if each LED is turned on and off individually (and in order)
void displayTime(uint8_t hour, uint8_t minute){
	custom_delay(brightnessLevel);
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
