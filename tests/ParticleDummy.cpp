#include "ParticleDummy.h"

// I/O Functions
void pinMode(uint16_t, uint8_t){}
uint16_t analogRead(uint16_t){return 0;}
uint16_t digitalReadd(uint16_t){return 0;}


// Timing Functions
uint32_t millis(void){return 0;}
void delayMicroseconds(uint32_t){}
void delay(uint32_t){}

//stuff
void SYSTEM_THREAD(bool value);

class Particle {
    public:
    uint32_t publish() {return 0;};

};


