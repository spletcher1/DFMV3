#include "GlobalIncludes.h"


#define OPTOPRESCALE               256
#define OPTOTOGGLES_PER_SEC        1000
#define OPTO_TICK               (GetPeripheralClock()/OPTOPRESCALE/OPTOTOGGLES_PER_SEC)

#define COL1ON() LATGSET=0x100
#define COL1OFF() LATGCLR=0x100
#define COL2ON() LATDSET=0x10
#define COL2OFF() LATDCLR=0x10

#define ROW1ON() LATFSET=0x40
#define ROW1OFF() LATFCLR=0x40
#define ROW2ON() LATDSET=0x80
#define ROW2OFF() LATDCLR=0x80
#define ROW3ON() LATDSET=0x40
#define ROW3OFF() LATDCLR=0x40
#define ROW4ON() LATDSET=0x100
#define ROW4OFF() LATDCLR=0x100
#define ROW5ON() LATGSET=0x40
#define ROW5OFF() LATGCLR=0x40
#define ROW6ON() LATGSET=0x80
#define ROW6OFF() LATGCLR=0x80

unsigned char volatile OptoState1;
unsigned char volatile OptoState2;
int optoOffThreshold;
int volatile optoOnCounter;
int volatile optoOffCounter;
unsigned int volatile pulseWidth_ms;
unsigned int volatile hertz;
char currentOptoTimerState;

void ConfigureOptoTimer(void);

void ConfigureOpto(){
   
    TRISFCLR = 0x40;
    TRISGCLR = 0x1C0;
    TRISDCLR = 0x1D0;
    
    ODCFSET = 0x40;
    ODCGSET = 0x1C0;
    ODCDSET = 0x1D0;
    
    LATFCLR = 0x40;
    LATGCLR = 0x1C0;
    LATDCLR = 0x1D0;
    
    OptoState1 = OptoState2 = 0;
    
    ConfigureOptoTimer();   
}

void Col1_Opto_On(){
    currentOptoTimerState=1;
    COL1ON();
    if(OptoState1 & 0x01)
         ROW1ON();
    if(OptoState1 & 0x02)
        ROW2ON();
    if(OptoState1 & 0x04)
        ROW3ON();
    if(OptoState1 & 0x08)
        ROW4ON();
    if(OptoState1 & 0x10) // need to change this back to 0x40 when new board is implemented.
        ROW5ON();
    if(OptoState1 & 0x20)
        ROW6ON();
}

void Col2_Opto_On(){
    currentOptoTimerState=2;
    COL2ON();
     if(OptoState2 & 0x01)
         ROW1ON();
    if(OptoState2 & 0x02)
        ROW2ON();
    if(OptoState2 & 0x04)
        ROW3ON();
    if(OptoState2 & 0x08)
        ROW4ON();
    if(OptoState2 & 0x10) // need to change this back to 0x40 when new board is implemented.
        ROW5ON();
    if(OptoState2 & 0x20)
        ROW6ON();
}

void inline Opto_Off(){
    currentOptoTimerState=0;
    LATFCLR = 0x40;
    LATGCLR = 0x1C0;
    LATDCLR = 0x1D0 ;
}

void SetOptoParameters(unsigned int hz, unsigned int pw) {
    unsigned char usingNewPort = 1;
    if (usingNewPort) {
        hertz = hz;
        if (hertz < 1) hertz = 1;
        if (hertz > 500) hertz = 500;
        pulseWidth_ms = pw;
        optoOffThreshold = (unsigned int) (1000 / hertz) - (pulseWidth_ms);
        // If the parameters don't work, then accept the hertz and
        // adjust the pulse width to be maximum given that hertz.
        if (optoOffThreshold < 0) {
            pulseWidth_ms = 1000 / (hertz * 2);
            optoOffThreshold = (unsigned int) (1000 / hertz) - (pulseWidth_ms);
        }
        optoOnCounter = optoOffCounter = 0;
    } else {
        hertz = hz;
        if (hertz < 1) hertz = 1;
        if (hertz > 250) hertz = 250;
        // Because we are multiplexing two rows, the pulse width of one
        // cycle is twice the pulse width defined for a single blink.
        // So we have to limit the hertz accordingly.
        pulseWidth_ms = pw;
        optoOffThreshold = (unsigned int) (1000 / hertz) - (pulseWidth_ms * 2);
        // If the parameters don't work, then accept the hertz and
        // adjust the pulse width to be maximum given that hertz.
        if (optoOffThreshold < 0) {
            optoOffThreshold = 0;
            pulseWidth_ms = 1000 / (hertz * 2);
        }
        optoOnCounter = optoOffCounter = 0;
    }
}

void SetPulseWidth_ms(unsigned int pw){
    SetOptoParameters(hertz,pw);
}

void SetOptoState(unsigned char os1, unsigned char os2){
    OptoState1=os1;
    OptoState2=os2;
}

void SetHertz(unsigned int hz){
  SetOptoParameters(hz,pulseWidth_ms);
}

void ConfigureOptoTimer(void) {
    // This timer is set to go off every 1ms.    
    //SetHertz(40);
    //SetHertz(100);
    //Set101();
    SetOptoParameters(2,200);
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, OPTO_TICK);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_4);          
    currentOptoTimerState=0;   
}
void __ISR(_TIMER_2_VECTOR, IPL4AUTO) Timer2Handler(void) {
    //PORTESET = 0x01;    
    if(currentOptoTimerState==0) {// All lights off
        optoOffCounter++;
        if(optoOffCounter>=optoOffThreshold) {   
            optoOffCounter=0;
            Col1_Opto_On();        
        }
    }
    else if(currentOptoTimerState==1) { //Column 1 lights attended to
        optoOnCounter++;
        if(optoOnCounter>=pulseWidth_ms) {
            Opto_Off();
            Col2_Opto_On();
            optoOnCounter=0;                  
        }
    }
    else if(currentOptoTimerState==2){ //Column 2 lights being attended to
        optoOnCounter++;
        if(optoOnCounter>=pulseWidth_ms) {
            Opto_Off();        
            optoOnCounter=0;                  
        }
    }       
    mT2ClearIntFlag();
}