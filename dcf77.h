#ifndef DCF77_H
#define DCF77_H
#define dcfInputPin PORTD3
#define dcfEnablePin PORTD4
#include "main.h"

extern volatile uint16_t ms;
extern bool transmissionStarted;
extern bool interpretationFinished;
extern volatile bool newSignal;
extern volatile uint16_t firstMeasurement;
extern volatile uint16_t measurement;
extern uint8_t digit;
extern uint8_t dcfBuffer[59];

extern bool interpretationSuccessful;

void initDCF77();
void waitForLowLevel();
void finitDCF77();
void interpretDcf77Signal();
void evaluateDcf77Signal();
uint8_t returnValue(uint8_t start, uint8_t end, bool isMinute);
uint8_t calculateBitSignificance(uint8_t bit, uint8_t relPos, bool isMinute);
bool checkParityBit(uint8_t value, uint8_t parityBit);

#endif