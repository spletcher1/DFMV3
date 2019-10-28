#include "GlobalIncludes.h"

#define PRESCALE               256
#define TOGGLES_PER_SEC        10
#define T1_TICK               (GetPeripheralClock()/PRESCALE/TOGGLES_PER_SEC)

int volatile HBCounter;
int volatile secondCounter;
unsigned char isInDarkMode;
unsigned char volatile timerFlag_100ms;
unsigned char volatile timerFlag_1sec;

void ConfigureUpdateTimer(void) {
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_256, T1_TICK);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
    HBCounter = 0;
    secondCounter=0;
}

// As of 6/17/2013 when reading 12 anolog inputs
// the interrupt takes about 200us to complete
// with ADCS = 0x06 and 2Tad aquisition time.
// with ADCS = 0x06 and 30Tad aquisition time it takes 300us.
// Maybe for now lets use 30Tad to insure sufficient aquisition.

// 7/1/2013
// I changed AD1CON3bits.SAMC = 0x1F and AD1CON3bits.ADCS = 0x2e
// to attempt to maximize aquisition time. This is 31Tad aquisition
// and a Tad = 92 TPb.
// Now this interrupt takes 1.35ms to complete. The interrupt goes off every
// 2.5ms so we should be okay.

// priority levels range from 1 (the lowest priority) to 7 (the highest)

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) Timer1Handler(void) {
    //PORTESET = 0x01;
    if (HBCounter % (TOGGLES_PER_SEC >> 1) == 0) {
        if (isInDarkMode == 0)
            FLIP_HEARTBEAT_LED();       
    }
    HBCounter++;
    if(secondCounter++>=10) {
        secondCounter=0;
        timerFlag_1sec=1;
    }
    timerFlag_100ms=1;
    mT1ClearIntFlag();
    //PORTECLR = 0x01;
}
