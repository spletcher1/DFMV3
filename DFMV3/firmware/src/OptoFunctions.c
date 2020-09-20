#include "GlobalIncludes.h"


#define OPTOPRESCALE               1
#define OPTOTOGGLES_PER_SEC        1000
#define OPTO_TICK               (GetPeripheralClock()/OPTOPRESCALE/OPTOTOGGLES_PER_SEC)


unsigned int volatile OptoState;
int volatile firstDCCounter;
int volatile secondDCCounter;

unsigned int volatile pulseWidth_ms;
unsigned int volatile hertz;
unsigned char volatile timerFlag_1ms;
unsigned char volatile timerFlag_200ms;
unsigned char volatile timer200msCounter;

unsigned int volatile optoPeriodCounter;
unsigned int volatile optoPeriod;

extern errorFlags_t volatile currentError;

// This parameter (UsingNewPortOnly) will be defined at startup and indicated by an LED, at the same time DFMID is determined.  
// To change it, the user will need to reset after the change.
extern unsigned char usingNewPort;
void ConfigureOptoTimer(void);

void ConfigureOpto() {
    /*
     * Note that these outputs are not configured by the MHC in the plib
     * initialization.  This will be removed from further versions.
    // Definitions for legacy port
    TRISFCLR = 0x40;
    TRISGCLR = 0x1C0;
    TRISDCLR = 0x1D0;

    ODCFSET = 0x40;
    ODCGSET = 0x1C0;
    ODCDSET = 0x1D0;

    LATFCLR = 0x40;
    LATGCLR = 0x1C0;
    LATDCLR = 0x1D0;

    // Definitions for new port
    // Note that RD4, RD6, RD7, RD8, and RD9 are present on both ports.
    // Rewriting these bits won't hurt...
    TRISDCLR = 0x0FFF;
    ODCDSET = 0xFFF;
    LATDCLR = 0xFFF;
    */
    OptoState= 0;    
    ConfigureOptoTimer();
}

void Col1_Opto_On() {    
    COL1ON();
    if (OptoState & 0x01)
        ROW1ON();
    if (OptoState & 0x04)
        ROW2ON();
    if (OptoState & 0x10)
        ROW3ON();
    if (OptoState & 0x40)
        ROW4ON();
    if (OptoState & 0x100) // need to change this back to 0x40 when new board is implemented.
        ROW5ON();
    if (OptoState & 0x400)
        ROW6ON();
}

void Col2_Opto_On() {    
    COL2ON();
    if (OptoState & 0x02)
        ROW1ON();
    if (OptoState & 0x08)
        ROW2ON();
    if (OptoState & 0x20)
        ROW3ON();
    if (OptoState & 0x80)
        ROW4ON();
    if (OptoState & 0x200) // need to change this back to 0x40 when new board is implemented.
        ROW5ON();
    if (OptoState & 0x800)
        ROW6ON();
}

void inline Opto_Off() {   
    // Legacy port definitions are combined with new port
    // here to avoid the cost of writing to the D port twice.
    LATFCLR = 0x40;
    LATGCLR = 0x1C0;
    LATDCLR = 0x3D0;            
}

void SetOptoParameters(unsigned int hz, unsigned int pw) {
    // Accept hertz up to 500.  If it is greater than 250, the legacy
    // port will not function
    hertz = hz;
    if (hertz < 1) {
        hertz = 1;
    }
    if(usingNewPort){
        if(hertz>500) 
            hertz = 500;
        if(pw > 1000/hertz)
            pulseWidth_ms = (1000/hertz)-1;     
        else 
            pulseWidth_ms = pw;    
    }
    else { 
        if(hertz>250) 
            hertz=250;
        if(pw > 1000/(hertz*2))
            pulseWidth_ms = (1000/(hertz*2));     
        else 
            pulseWidth_ms = pw;    
    }        
    firstDCCounter = 0;
    secondDCCounter = 0;  
    optoPeriod = 1000/hertz;
}

void SetPulseWidth_ms(unsigned int pw) {
    SetOptoParameters(hertz, pw);
}

void inline SetOptoState(unsigned int os) {
    OptoState = os;    
}

void SetHertz(unsigned int hz) {
    SetOptoParameters(hz, pulseWidth_ms);
}

// This interrupt is used when the new port is active
void TIMER2_EventHandlerNewPort(uint32_t status, uintptr_t context) {          
    // First set flags, checking for errors
    if(timerFlag_1ms==1){
        BLUELED_ON();    
        currentError.bits.INTERRUPT=1;       
    }
    timerFlag_1ms = 1; 
    
    if(timer200msCounter++>=200){       
        if(timerFlag_200ms==1){
            BLUELED_ON();    
            currentError.bits.INTERRUPT=1;       
        }
        timerFlag_200ms=1;
        timer200msCounter=0;        
    }
    
    // Not reset period counter if neeeded
    if(++optoPeriodCounter>=optoPeriod){
        firstDCCounter=0;
        optoPeriodCounter=0;        
    }
    
    if(firstDCCounter++<pulseWidth_ms)
        LATD = OptoState;
    else
        LATDCLR = 0xFFF;
}

// This interrupt is used when the old port is active
void TIMER2_EventHandlerOldPort(uint32_t status, uintptr_t context) {          
    // First set flags, checking for errors
    if(timerFlag_1ms==1){
        BLUELED_ON();    
        currentError.bits.INTERRUPT=1;       
    }
    timerFlag_1ms = 1; 
    
    if(timer200msCounter++>=200){       
        if(timerFlag_200ms==1){
            BLUELED_ON();    
            currentError.bits.INTERRUPT=1;       
        }
        timerFlag_200ms=1;
        timer200msCounter=0;        
    }
    
    // Not reset period counter if neeeded
    if(++optoPeriodCounter>=optoPeriod){
        firstDCCounter=0;
        secondDCCounter=0;
        optoPeriodCounter=0;        
    }
    
    Opto_Off();    
    if(firstDCCounter++<pulseWidth_ms)
        Col1_Opto_On();   
    else if(secondDCCounter++<pulseWidth_ms)
        Col2_Opto_On();        
}

void TogglePortUse(){
     if(usingNewPort==1){
        usingNewPort=0;
        YELLOWLED_ON();
        TMR2_CallbackRegister(TIMER2_EventHandlerOldPort,(uintptr_t)NULL);
    }
    else {
        usingNewPort=1;
        YELLOWLED_OFF();
        TMR2_CallbackRegister(TIMER2_EventHandlerNewPort,(uintptr_t)NULL);
    }
    SetOptoParameters(hertz, pulseWidth_ms);    
}
void ConfigureOptoTimer(void) {
    // This timer is set to go off every 1ms.    
    //SetHertz(40);
    //SetHertz(100);
    //Set101();
    SetOptoParameters(40, 8);           
    OptoState=0x00;
    firstDCCounter = secondDCCounter=0;
    timerFlag_1ms = 0;
    optoPeriodCounter=0;
    timerFlag_200ms=timer200msCounter=0;
    if(usingNewPort)
        TMR2_CallbackRegister(TIMER2_EventHandlerNewPort,(uintptr_t)NULL);
    else
        TMR2_CallbackRegister(TIMER2_EventHandlerOldPort,(uintptr_t)NULL);
    TMR2_Start();
}
