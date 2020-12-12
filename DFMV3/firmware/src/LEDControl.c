#include "GlobalIncludes.h"


int LEDDecayCounter[12];
int LEDDelayCounter[12];
int LEDMaxTimeOnCounter[12];

unsigned int LEDDecayValues[12];
unsigned int LEDDelayValues[12];
unsigned int LEDMaxTimeOnValues[12];

LEDFLAGS IsLEDOn;
LEDFLAGS LEDLinkFlags[12];
unsigned char IsOverMaxTimeOn[12];

extern int CurrentValues[13]; // This has one extra place for the input voltage reading from the DFM.
int LEDThresholdValues[12];

void (*LEDUpdateFunction)(unsigned char);
void SetLEDParams(unsigned int decayval, unsigned int delayval, unsigned int maxtimeonval);

// For now we will force each well on a DFM to have the same delay, decay, maxtime values
unsigned int currentDelay;
unsigned int currentDecay;
unsigned int currentMaxTimeOn;


void ClearLEDLinkFlags(){
    int i;
    for(i=0;i<12;i++){
        LEDLinkFlags[i].ledField=(1<<i);
    }    
}

// Link defs will be an array of length 12, one for each LED
// LEDs with the same number will be linked.  Max number is 12.
void SetLEDLinkFlags(unsigned char *linkdefs){
    int i,j;
    char currentLinkNumber;
    for(i=0;i<12;i++){
        currentLinkNumber=linkdefs[i];
        LEDLinkFlags[i].ledField = 0;
        for(j=0;j<12;j++){
            if(linkdefs[j]==currentLinkNumber){
                LEDLinkFlags[i].ledField |= (1<<j);    
            }
        }
    }
}



void InitializeLEDControl(unsigned int decayval,unsigned int delayval,unsigned int maxtimeval) {    
    int i; 
    for(i=0;i<12;i++)
        LEDThresholdValues[i]=-1;       
    ClearLEDLinkFlags();    
    SetLEDParams(decayval,delayval,maxtimeval);
}

void ClearLEDThresholds(){
    int i;
    for(i=0;i<12;i++)
        LEDThresholdValues[i]=-1;
}

void TestLEDThresholds(){
    int i;
    for(i=0;i<12;i++)
        LEDThresholdValues[i]=100*128;
}

 // Note that currently (12/11/20) a -1 is transmitted and received as a two byte integer == 0xFFFF = 65535.
 // So thresh[i] will never be -1 as received from the MCU.  That is okay, because the -1 turns into a threshold so high
 // that it can never turn on the LEDs.  Nevertheless, the -1 conditions in the LEDControl.c code is unneeded.
// I will change the condition to search > 2000.
void SetLEDThresholds(int *thresh){
    int i;
    for(i=0;i<12;i++){
        if(thresh[i]>2000) 
            LEDThresholdValues[i]=-1;
        else
            LEDThresholdValues[i]=thresh[i]*128;
    }
}

void inline SetLEDOn(unsigned char led){    
    // Set current and linked LEDs here.  
    IsLEDOn.ledField |= LEDLinkFlags[led].ledField; 
}

//void inline SetLEDOff(unsigned char led){
//    IsLEDOn.ledField &= ~(1<<led);
//}

void UpdateLEDSimplest(unsigned char led) {
    if (LEDThresholdValues[led] == -1) { 
        return;
    }  
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        SetLEDOn(led);
        return;
    } 
}

void UpdateLEDWithDecay(unsigned char led) {
    if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }   
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        SetLEDOn(led);
        LEDDecayCounter[led] = LEDDecayValues[led];
    } else if (LEDDecayValues[led] > 0 && LEDDecayCounter[led] > 0) {
        LEDDecayCounter[led]--;
        SetLEDOn(led);
    } 
}


void UpdateLEDFixedInterval(unsigned char led) {
    if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }   
    
    // This first defines what happens outside T
    if (LEDDelayCounter[led] <= 0) {
        if (CurrentValues[led] > LEDThresholdValues[led]) {  // Should be first lick outside T. Turn on light and start both timers.
            SetLEDOn(led);
            LEDDecayCounter[led] = LEDDecayValues[led];
            LEDDelayCounter[led] = LEDDelayValues[led];            
        }
        else {
            LEDDelayCounter[led]=-1; // Here if outside T and no lick...just hang out. Keep LED Off.
        }
    }    
    else { // This part defines what happens inside T.
        LEDDelayCounter[led]--;
        if (LEDDecayCounter[led] > 0) { // Inside T but also inside decay, set LED on.
            LEDDecayCounter[led]--;
            SetLEDOn(led);
        } 
        else {
            LEDDecayCounter[led]=-1; // If inside T but outside the decay...stay off            
        }
    }
}

void UpdateLEDWithDecayAndMaxTime(unsigned char led) {
    if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    } 
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        if (LEDDecayValues[led] > 0) {
            LEDDecayCounter[led] = LEDDecayValues[led];
        }
        if (LEDMaxTimeOnValues[led] > 0) {
            if (LEDMaxTimeOnCounter[led]-- > 0) {
                SetLEDOn(led);
            } 
        } else {
            SetLEDOn(led);
        }

    } else {
        if (LEDDecayValues[led] > 0) {
            if (LEDDecayCounter[led]-- > 0) {
                if (LEDMaxTimeOnValues[led] > 0) {
                    if (LEDMaxTimeOnCounter[led]-- > 0) {
                        SetLEDOn(led);
                    } 
                } else {
                    SetLEDOn(led);
                }
            } else {                
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        } else {            
            LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
        }
    }
}

// This function is ONLY used when delay > 0.
// We ignore maxtime on for this because the lights on time
// is determined uniquely by the decay time.
// We also require a decay > 0.

void UpdateLEDWithDelay(unsigned char led) {
     if (LEDThresholdValues[led] == -1) {    
        return;
    }
    if (LEDThresholdValues[led] == 0) {
        SetLEDOn(led);
        return;
    }
   
    if (CurrentValues[led] > LEDThresholdValues[led]) {        
        LEDDelayCounter[led] = LEDDelayValues[led];
        LEDDecayCounter[led] = LEDDecayValues[led];
    } 
    else {
        if (LEDDelayCounter[led]-- > 0) {            
            LEDDecayCounter[led] = LEDDecayValues[led];
        } 
        else {
            if (LEDDecayCounter[led]-- >= 0) {
                SetLEDOn(led);
            } else {                
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        }
    }
}



// Note that one has to be careful to set the parameters in the right manner
// or constraints may result in some unintended changes.
void SetLEDParams(unsigned int decayval, unsigned int delayval, unsigned int maxtimeonval) {
    int i;
    if(delayval>0){
        maxtimeonval=0;
        if(decayval<=0)
            decayval=1000;
    }        
    for (i = 0; i < 12; i++) {
        LEDDecayValues[i] = decayval;
        LEDDelayValues[i] = delayval;
        LEDMaxTimeOnValues[i] = maxtimeonval;

        LEDDecayCounter[i] = 0;
        if(delayval>0)
            LEDDelayCounter[i] = 0;
        else
            LEDDelayCounter[i] = LEDDelayValues[i];
        
        LEDMaxTimeOnCounter[i] = LEDMaxTimeOnValues[i];        
    }
    IsLEDOn.ledField=0;
    if (delayval == 0 && maxtimeonval == 0 && decayval == 0) {
        LEDUpdateFunction = &UpdateLEDSimplest;
    } else if (delayval == 0 && maxtimeonval == 0) {
        LEDUpdateFunction = &UpdateLEDWithDecay;
    } else if (delayval == 0) {
        LEDUpdateFunction = &UpdateLEDWithDecayAndMaxTime;
    } else if(maxtimeonval==0){
        LEDUpdateFunction = &UpdateLEDFixedInterval;
    }
    else {
        LEDUpdateFunction = &UpdateLEDWithDelay;
    }
    currentDelay = delayval;
    currentDecay = decayval;
    currentMaxTimeOn =maxtimeonval; 
}

void SetDelay(unsigned int delay){
    SetLEDParams(currentDecay,delay,currentMaxTimeOn);
}

void SetDecay(unsigned int decay){
    SetLEDParams(decay,currentDelay,currentMaxTimeOn);
}

void SetMaxTimeOn(unsigned int maxTime){
    SetLEDParams(currentDecay,currentDelay,maxTime);
}

void StepLEDControl() {
    unsigned char i;
    // We now start by assuming everyone is off.
    IsLEDOn.ledField=0;
    for (i = 0; i < 12; i++){               
        // LED Update only sets those as on.
        LEDUpdateFunction(i);        
    }    
    SetOptoState(IsLEDOn.ledField);
}
