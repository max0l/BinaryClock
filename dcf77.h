#pragma once
#define dcfInputPin PORTD3
#define dcfEnablePin PORTD4
#include "main.h"

volatile uint32_t ms;
bool transmissionStarted;
bool interpretationFinished;
volatile bool newSignal;
volatile uint32_t firstMeasurement;
volatile uint32_t lastMesurement;
volatile uint32_t measurement;
uint8_t digit;
uint8_t dcfBuffer[59];
volatile uint8_t periodCount;
uint8_t tryCounter;

bool interpretationSuccessful;

void initDCF77();
void finitDCF77();
void interpretDcf77Signal();
bool evaluateDcf77Signal();
uint8_t returnValue(uint8_t start, uint8_t end, bool isMinute);
uint8_t calculateBitSignificance(uint8_t bit, uint8_t relPos);
bool checkParityBit(uint8_t value, uint8_t parityBit);
bool waitForStartSequence();
bool checkMesurement(uint32_t rangeStart, uint32_t rangeEnd);
void takeMeasurement();
void returnToMain();