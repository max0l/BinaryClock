#include "dcf77.h"
#include "main.h"

volatile uint16_t ms = 0;
bool transmissionStarted = false;
bool interpretationFinished = false;
enum Level currentLevel = LOW;
volatile uint16_t firstMeasurement = 0;
volatile uint16_t measurement = 0;
volatile bool newSignal = false;
uint8_t digit = 0;
uint8_t dcfBuffer[59] = {0};

uint8_t errors = 0;

bool interpretationSuccessful = false;

void initDCF77() {
    sleepEnabled = false;
    
    //Set dcf66 Enable pin to low to enable the dcf77 module (deaktiviere Pullup) --> trying to set it as an output pin and low
    PORTD &= ~(1 << dcfEnablePin);

    PORTD &= ~(1 << dcfInputPin);
    
    //Set the interrupt to trigger on any change
    EICRA |= (1 << ISC10);
    //Enable the interrupt for the dcf77 module
    EIMSK |= (1 << INT1);
    

    //setup of the timer for the dcf77 module
    //0 -> 100 ms, 1 -> 200 ms. Need to find good prescaler for ms
    TCCR0B |= (1<<CS01); //this is the prescaler, needs to be set to 128
	OCR0A = 125-1; // this is the value that the timer counts to (and then resets to 0)
	TCCR0A |= (1<<WGM01); //enable CTC -> Timer wird zurückgesetzt wenn OCR0A erreicht wird
	TIMSK0 |= (1<<OCIE0A); //enable compare Interrupt 0A (of OCR0A)
    displayTime(48, 63);
    sei();
    interpretDcf77Signal();
}



void interpretDcf77Signal() {
    //PORTD |= (1 << PD5);

    waitingForLowLevelAndStartSequence();
    //PORTD &= ~(1 << PD5);
    //PORTD |= (1 << PD5);
    switchLevel(HIGH);
    //PORTD &= ~(1 << PD5);
    while(!interpretationFinished) {
        //PORTD |= (1 << PD5);
        if(newSignal == false) {
            continue;
        }

        if (checkMesurement((uint16_t) 50, (uint16_t) 150)) {
            dcfBuffer[digit] = 0;
            digit++;
            
            if(PIND & (1<<PD5)) {
                PORTD &= ~(1 << PD5);
            } else {
                PORTD |= (1 << PD5);
            }
            
            
        }

        if (checkMesurement((uint16_t) 150, (uint16_t) 250)) {
            dcfBuffer[digit] = 1;
            digit++;
            
            if(PIND & (1<<PD5)) {
                PORTD &= ~(1 << PD5);
            } else {
                PORTD |= (1 << PD5);
            }
            
        }

        if(digit == 58) {
            interpretationFinished = true;
            newSignal = false;
            transmissionStarted = false;
        }
        //PORTD &= ~(1 << PD5);
    }
    
    finitDCF77();
}

void waitingForLowLevelAndStartSequence() {
    
    switchLevel(LOW);
    
    waitForStartSequence();
}

void waitForStartSequence() {
    while(!transmissionStarted) {
        if(checkMesurement((uint16_t) 1000, (uint16_t) 2000)) {
            transmissionStarted = true;
        }
    }

}

bool checkMesurement(uint16_t rangeStart, uint16_t rangeEnd) {
    if(measurement > rangeStart && measurement <= rangeEnd && newSignal) {
        measurement = 0;
        newSignal = false;
        return true;
    } else {
        return false;
    }
}

void finitDCF77() {
    if(!evaluateDcf77Signal()) {
        digit = 0;
        interpretationFinished = false;
        transmissionStarted = false;
        interpretDcf77Signal();
    }
    //disables everything what the dcf77 needs
    PORTD |= (1 << dcfEnablePin); //activate pull up
    EICRA &= ~((1 << ISC11) | (1 << ISC10)); 
    EIMSK &= ~(1 << INT1); 
    //disable the timer for the dcf77
    TCCR0B = 0;
    TCCR0A = 0;
    TIMSK0 = 0;

    displayTime(48, 63);
    PORTD &= ~(1 << PD5);
    sleepEnabled = true;
}

//Pin 1 Interrupt on change
ISR(INT1_vect){ 
    takeMeasurement();
}



void switchLevel(enum Level wantedLevel) {
    //disable the interrupt for the dcf77 module
    //If the level LOW is wanted, i need to wait for the next HIGH level since
    //the next mesurement will be taking on falling and then rising edge
    
    if(wantedLevel == HIGH) {
        
        while(dcfInputPinIsHigh()) {
            asm("nop");
        }
        
        currentLevel = HIGH;
    } else if(wantedLevel == LOW) {
        
        while(!dcfInputPinIsHigh()) {
            asm("nop");
        }
        currentLevel = LOW;
        
    }
    
    //enable the interrupt for the dcf77 module
    ms = 0;
    firstMeasurement = 0;
    measurement = 0;
}

bool dcfInputPinIsHigh() {
    return PIND & (1 << dcfInputPin);
}

void takeMeasurement() {
    if(firstMeasurement == 0) {
        firstMeasurement = ms;
    } else {
        volatile uint16_t currentMs = ms;
        if(currentMs < firstMeasurement) {
            currentMs = UINT16_MAX - firstMeasurement + currentMs;
        }
        measurement = currentMs - firstMeasurement;
        firstMeasurement = 0;
        newSignal = true;
    }
}

//timer, counts ms
ISR(TIMER0_COMPA_vect) {
    if(ms == UINT16_MAX) {
        ms = 0;
    } else {
        ms++;
    }
}

bool evaluateDcf77Signal() {
    bool signalOk = true;
    uint8_t dcf77Hour = returnValue(29, 35, false);
    uint8_t dcf77Minute = returnValue(21, 28, true);

    if(checkParityBit(29,35)) {
        hour = dcf77Hour;
    } else {
        //hour = 4;
        signalOk = false;
    }

    if(checkParityBit(21,28)) {
        minute = dcf77Minute;
    } else {
        //minute = 4;
        signalOk = false;
    }
    return signalOk;
}

uint8_t returnValue(uint8_t start, uint8_t end, bool isMinute) {
    uint8_t value = 0;
    for (size_t i = start; i < end; i++)
    {
        value += calculateBitSignificance(dcfBuffer[i], i, isMinute);
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

bool checkParityBit(uint8_t rangeStart, uint8_t rangeEnd) {
    uint8_t sum = 0;
    for (size_t i = rangeStart; i < rangeEnd; i++)
    {
        sum += dcfBuffer[i];
    }
    if(sum % 2 == 0) {
        return true;
    } else {
        return false;
    }
}