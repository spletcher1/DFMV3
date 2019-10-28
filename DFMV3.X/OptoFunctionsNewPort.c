#include "GlobalIncludes.h"

#define CHAMBER1_ON() LATDbits.LATD0=1
#define CHAMBER2_ON() LATDbits.LATD1=1
#define CHAMBER3_ON() LATDbits.LATD2=1
#define CHAMBER4_ON() LATDbits.LATD3=1
#define CHAMBER5_ON() LATDbits.LATD4=1
#define CHAMBER6_ON() LATDbits.LATD5=1
#define CHAMBER7_ON() LATDbits.LATD6=1
#define CHAMBER8_ON() LATDbits.LATD7=1
#define CHAMBER9_ON() LATDbits.LATD8=1
#define CHAMBER10_ON() LATDbits.LATD9=1
#define CHAMBER11_ON() LATDbits.LATD10=1
#define CHAMBER12_ON() LATDbits.LATD11=1

#define CHAMBER1_OFF() LATDbits.LATD0=0
#define CHAMBER2_OFF() LATDbits.LATD1=0
#define CHAMBER3_OFF() LATDbits.LATD2=0
#define CHAMBER4_OFF() LATDbits.LATD3=0
#define CHAMBER5_OFF() LATDbits.LATD4=0
#define CHAMBER6_OFF() LATDbits.LATD5=0
#define CHAMBER7_OFF() LATDbits.LATD6=0
#define CHAMBER8_OFF() LATDbits.LATD7=0
#define CHAMBER9_OFF() LATDbits.LATD8=0
#define CHAMBER10_OFF() LATDbits.LATD9=0
#define CHAMBER11_OFF() LATDbits.LATD10=0
#define CHAMBER12_OFF() LATDbits.LATD11=0

unsigned char volatile OptoState1;
unsigned char volatile OptoState2;
int volatile opto_msOFF;
unsigned int volatile optoOnCounter;
unsigned int volatile optoOffCounter;
unsigned int volatile pulseWidth_ms;
unsigned int volatile hertz;
unsigned char isOptoOn;
int hbCounter;


void Opto_On() {
    isOptoOn = 1;
    if (OptoState1 & 0x01)
        CHAMBER1_ON();
    if (OptoState1 & 0x02)
        CHAMBER3_ON();
    if (OptoState1 & 0x04)
        CHAMBER5_ON();
    if (OptoState1 & 0x08)
        CHAMBER7_ON();
    if (OptoState1 & 0x10) // need to change this back to 0x40 when new board is implemented.
        CHAMBER9_ON();
    if (OptoState1 & 0x20)
        CHAMBER11_ON();
    if (OptoState2 & 0x01)
        CHAMBER2_ON();
    if (OptoState2 & 0x02)
        CHAMBER4_ON();
    if (OptoState2 & 0x04)
        CHAMBER6_ON();
    if (OptoState2 & 0x08)
        CHAMBER8_ON();
    if (OptoState2 & 0x10) // need to change this back to 0x40 when new board is implemented.
        CHAMBER10_ON();
    if (OptoState2 & 0x20)
        CHAMBER12_ON();

}

void inline Opto_Off() {
    isOptoOn = 0;
    CHAMBER1_OFF();
    CHAMBER2_OFF();
    CHAMBER3_OFF();
    CHAMBER4_OFF();
    CHAMBER5_OFF();
    CHAMBER6_OFF();
    CHAMBER7_OFF();
    CHAMBER8_OFF();
    CHAMBER9_OFF();
    CHAMBER10_OFF();
    CHAMBER11_OFF();
    CHAMBER12_OFF();
}


