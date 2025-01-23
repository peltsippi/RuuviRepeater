
#ifndef PARTICLEDUMMY_H
#define PARTICLEDUMMY_H

#include "stdint.h"


#define A0 160
#define A1 161
#define A2 162
#define A3 163
#define A4 164
#define A5 165
#define A6 166
#define A7 167

#define BATT 31
#define CHG 44



#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1


// I/O Functions
void pinMode(uint16_t, uint8_t);
uint16_t analogRead(uint16_t);
uint16_t digitalReadd(uint16_t);


//void SYSTEM_THREAD(bool);

// Timing Functions
uint32_t millis(void);
void delayMicroseconds(uint32_t);

void delay(uint32_t);




#endif
