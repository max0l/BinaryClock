#include "dcf77.h"
#include "main.h"

volatile uint16_t ms = 0;
bool transmissionStarted = false;
bool interpretationFinished = false;
volatile uint16_t firstMeasurement = 0;
volatile uint16_t measurement = 0;
volatile bool newSignal = false;
uint8_t digit = 0;
uint8_t dcfBuffer[59] = {0};

uint8_t errors = 0;

bool interpretationSuccessful = false;

void initDCF77() {
    sleepEnabled = false;
    
    //Set dcf66 Enable pin to low to enable the dcf77 module (deaktiviere Pullup)
    PORTD &= ~(0 << dcfEnablePin);

    //PORTD |= (1 << dcfInputPin); //activate pull up
    
    //Set the interrupt to trigger on any change
    EICRA |= (1 << ISC10);
    //Enable the interrupt for the dcf77 module
    EIMSK |= (1 << INT1);
    

    //setup of the timer for the dcf77 module
    //0 -> 100 ms, 1 -> 200 ms. Need to find good prescaler for ms
    TCCR0B |= (1<<CS01); //this is the prescaler, needs to be set to 128
	OCR0A = 125-1; // this is the value that the timer counts to (and then resets to 0)
	TCCR0A |= (1<<WGM01); //enable CTC -> Timer wird zurückgesetzt wenn OCR0A erreicht wird
	TIMSK0 |= (1<<OCIE0A); //enable compare Interrupt 1A (of OCR0A)
    
    sei();
    waitForLowLevel();
    interpretDcf77Signal();
}

void waitForLowLevel() {
    while((PIND & (1 << dcfInputPin))) {
        asm("nop");
    }
}


void interpretDcf77Signal() {
    while(!interpretationFinished) {
        if(measurement == 0) {
            continue;
        }
        if (measurement > 0 && measurement < 50 && newSignal) {
            interpretationSuccessful = false;
            transmissionStarted = false;
            ms = 0;
            digit = 0;
            newSignal = false;
            errors++;
            displayTime(0, 32);
        } else if (measurement >= 50 && measurement <= 150 && transmissionStarted && newSignal) {
            dcfBuffer[digit] = 0;
            digit++;
            measurement = 0;
            newSignal = false;
            //displayTime(0,4);
        } else if (measurement > 150 && measurement <= 250 && transmissionStarted && newSignal) {
            dcfBuffer[digit] = 1;
            digit++;
            measurement = 0;
            newSignal = false;
            //displayTime(0,4);
        } else if(measurement > 250 && measurement <= 2000 && newSignal) {
            if(transmissionStarted && digit == 58) {
                interpretationFinished = true;
                interpretationSuccessful = true;
                //displayTime(0, 8);
            } else if(!transmissionStarted) {
                transmissionStarted = true;
                ms = 0;
                digit = 0;
                //displayTime(0,2);
            }
            newSignal = false;
        } else  if(measurement >= 2000 && newSignal) {
            interpretationFinished = true;
            interpretationSuccessful = false;
            displayTime(0, 48);
        }
        if((measurement > 10000 && !transmissionStarted) || errors > 10) {
            displayTime(0, 48);
            interpretationFinished = true;
            interpretationSuccessful = false;
            
        }
    }
    finitDCF77();
}


void finitDCF77() {
    //disables everything what the dcf77 needs
    PORTD |= (1 << dcfEnablePin); //activate pull up
    EICRA &= ~((1 << ISC11) | (1 << ISC10)); 
    EIMSK &= ~(1 << INT1); 
    //disable the timer for the dcf77
    TCCR0B = 0;
    TCCR0A = 0;
    TIMSK0 = 0;

    
    if(interpretationSuccessful) {
        evaluateDcf77Signal();
    } else {
        minute = 3;
        hour = 3;
    }
    sleepEnabled = true;
}

//Pin 1 Interrupt on change
ISR(INT1_vect){ 
    if(firstMeasurement == 0) {
        firstMeasurement = ms;
        //displayTime(0, (uint8_t) firstMeasurement);
    } else {
        measurement = ms - firstMeasurement;
        //displayTime((uint8_t) measurement, 0);
        firstMeasurement = 0;
        newSignal = true;
    }
    //displayTime(0, 1);
}


//timer, counts ms
ISR(TIMER0_COMPA_vect) {
    /*
    if(PORTB & (1 << PORTB0)) {
        PORTB &= ~(1 << PORTB0);
    } else {
        PORTB |= (1 << PORTB0);
    }
    */
    if(ms == 0xFFFF) {
        ms = 0;
    } else {
        ms++;
    }
}

void evaluateDcf77Signal() {
    uint8_t dcf77Hour = returnValue(29, 35, false);
    uint8_t dcf77Minute = returnValue(21, 28, true);

    if(checkParityBit(dcf77Hour, dcfBuffer[35])) {
        hour = dcf77Hour;
    } else {
        hour = 4;
    }

    if(checkParityBit(dcf77Minute, dcfBuffer[28])) {
        minute = dcf77Minute;
    } else {
        minute = 4;
    }
}

uint8_t returnValue(uint8_t start, uint8_t end, bool isMinute) {
    uint8_t value = 0;
    for (size_t i = start; i < end; i++)
    {
        value += calculateBitSignificance(dcfBuffer[i], i,isMinute);
    }

    

    return value;
}

uint8_t calculateBitSignificance(uint8_t bit, uint8_t relPos, bool isMinute) {
    uint8_t position;
    uint8_t significance[] = {1, 2, 4, 8, 10, 20, 40};
    if(isMinute) {
        position = relPos - 21;
    } else {
        position = relPos - 29;
    }
    return bit * significance[position];
}

bool checkParityBit(uint8_t value, uint8_t parityBit) {
    uint8_t parity = 0;
    for (size_t i = 0; i < 6; i++)
    {
        if(value & (1 << i)) {
            parity++;
        }
    }
    if(parity % 2 == 0) {
        return false;
    } else {
        return true;
    }
}