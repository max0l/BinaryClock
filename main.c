#include "main.h"
#include "dcf77.h"

volatile uint8_t hour = 1;
volatile uint8_t minute = 1;

volatile uint16_t second = 0;

const uint8_t buttons = (1 << PD0) | (1 << PD1) | (1 << PD2); 

volatile uint8_t prell = 0;
volatile uint8_t brightnessLevel = 0;
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

//drift
volatile uint8_t negateCounter = 0;


/////////////////////////////////

int main() {
	// Setze alle Pins von Port C als Ausgänge
	DDRC = 0b00111111;
	DDRD = 0b11110000;
	DDRB = 0b00000111;
	
	//////////////////////////////////////////////
	//Buttons
	// activet pull down for buttons
	PORTD |= buttons;

	initDCF77();


	//Power save
	power_adc_disable();
	power_usart0_disable();
	power_spi_disable();
	power_twi_disable();
	power_timer1_disable();
	
	//Trigger one IO Chnage on INT0 (falling edge)
	EICRA |= (1<<ISC01);
	
	//Enable Button1 Interrupt INT0
	EIMSK |= (1<<INT0); 
	
	////////////////////////////////////////
	//Clock
	//Set AS2 to 1 so TSK1 and TASK2 (external quartz clock)
	ASSR |= (1<<AS2);
	
	//Timer Interrupts
	TCCR2B |= (1<<CS02) | (1<<CS20); //this is the prescaler, needs to be set to 128
	OCR2A = 256-1; // this is the value that the timer counts to
	TCCR2A |= (1<<WGM01); //enable CTC -> Timer will reset when OCR2A is reached
	TIMSK2 |= (1<<OCIE2A); //enable compare Interrupt 1A (of OCR0A)
	

	//Set Sleep mode
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	
	sei();

		
	//Main Loop
	while (1)
	{	
		
		if(currentState == SLEEP_MODE && sleepEnabled) {
			sleep_mode();
		} else {
			checkButtons();
			switch(currentState) {
				case DISPLAY_TIME:
					displayTime(hour, minute);
					break;
				case SET_HOUR:
					displayTime(hour, 63);
					break;
				case SET_MINUTE:
					displayTime(31, minute);
					break;
				case ADJUST_BRIGHTNESS:
					displayTime(31, 63);
					break;
				default:
					displayTime(hour, minute);
					break;
			}
		}
	}
	return 0;
}

ISR(INT0_vect) {
	switch(currentState) {
	case DISPLAY_TIME:
		sleepButton();
		break;
	case SET_HOUR:
		alterHour(-1);
		break;
	case SET_MINUTE:
		alterMinute(-1);
		break;
	case ADJUST_BRIGHTNESS:
		adjustBrightnes(-1);
		break;
	case SLEEP_MODE:
		sleepButton();
		break;
	default:
		sleepButton();
		break;
	}
}

void checkButtons() {
	//debounce
	if(prell) {
		prell--;
		return;
	}
	prell = 7;


    if (!(PIND & (1 << PD0))) {

    	switch(currentState) {
			case DISPLAY_TIME:
			    currentState = SET_HOUR;
				break;
			case SET_HOUR:
				currentState = SET_MINUTE;
				break;
			case SET_MINUTE:
				currentState = DISPLAY_TIME;
				break;
			case ADJUST_BRIGHTNESS:
				currentState = DISPLAY_TIME;
				break;
			case SLEEP_MODE:
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
	
}

//Timer0 Interrupt
ISR(TIMER2_COMPA_vect) {
	second++;
	if(second == 60){
		second = 0;
		minute++;
		if(minute == 60){
			minute = 0;
			hour++;
			negateCounter++;
			if(hour == 24){
				hour = 0;
			}
		}
	}
	/* Debugging
	if(PIND & (1<<PD5)){
		PORTD &= ~(1 << PD5);
	} else {
		PORTD |= (1 << PD5);
	}
	*/

	//Negating Drift
	if(negateCounter == NEGATEDRIFT){
		second++;
		negateCounter = 0;
	}

	//Sleep Mode downcounter
	if(currentState == DISPLAY_TIME && sleepEnabled){
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
	if(value < 0 && minute > 0) {
		minute--;
		return;
	}
	if(value > 0 && minute == 59) {
		minute = 0;
		return;
	}
	if(value > 0 && minute < 59) {
		minute++;
		return;
	}
}

void alterHour(int8_t value) {
	if(value<0 && hour == 0){
		hour = 23;
		return;
	}
	if(value < 0 && hour > 0) {
		hour--;
		return;
	}
	if(value > 0 && hour == 23) {
		hour = 0;
		return;
	}
	if(value > 0 && hour < 23 ) {
		hour++;
		return;
	}
}

void adjustBrightnes(int8_t value) {
	if(value < 0 && brightnessLevel == 0) {
		brightnessLevel = 4;
		return;
	}
	if(value < 0 && brightnessLevel > 0) {
		brightnessLevel--;
		return;
	}
	if(value > 0 && brightnessLevel == 4) {
		brightnessLevel = 0;
		return;
	}
	if(value > 0 && brightnessLevel < 4) {
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
