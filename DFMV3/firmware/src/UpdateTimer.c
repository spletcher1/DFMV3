#include "GlobalIncludes.h"

#define TOGGLES_PER_HALFSEC    5


int volatile HBCounter;
int volatile secondCounter;
extern unsigned char isInDarkMode;
unsigned char volatile timerFlag_1sec;


void TIMER1_EventHandler(uint32_t status, uintptr_t context) {     
    if (HBCounter >= TOGGLES_PER_HALFSEC) {
        //if (isInDarkMode == 0)
        //    FLIP_HEARTBEAT_LED();       
        HBCounter=0;
    }
    HBCounter++;
    if(secondCounter++>=10) {
        secondCounter=0;
        timerFlag_1sec=1;         
    }    
}

void ConfigureUpdateTimer(void) {   
    TMR1_CallbackRegister(TIMER1_EventHandler,(uintptr_t)NULL);
    TMR1_Start();
    HBCounter = 0;
    secondCounter=0;    
}



