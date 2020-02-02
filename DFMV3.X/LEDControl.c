
#include "LEDControl.h"


int LEDDecayCounter[12];
int LEDDelayCounter[12];
int LEDMaxTimeOnCounter[12];

unsigned int LEDDecayValues[12];
unsigned int LEDDelayValues[12];
unsigned int LEDMaxTimeOnValues[12];

LEDFLAGS IsLEDOn;
unsigned char IsOverMaxTimeOn[12];

extern int CurrentValues[13]; // This has one extra place for the input voltage reading from the DFM.
int LEDThresholdValues[12];

void (*LEDUpdateFunction)(unsigned char);
void SetLEDParams(unsigned int decayval, unsigned int delayval, unsigned int maxtimeonval);

// For now we will force each well on a DFM to have the same delay, decay, maxtime values
unsigned int currentDelay;
unsigned int currentDecay;
unsigned int currentMaxTimeOn;


void InitializeLEDControl(unsigned int decayval,unsigned int delayval,unsigned int maxtimeval) {    
    int i;
    for(i=0;i<12;i++)
        LEDThresholdValues[i]=-1;
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

void SetLEDThresholds(int *thresh){
    int i;
    for(i=0;i<12;i++)
        LEDThresholdValues[i]=thresh[i]*128;
}

void UpdateLEDSimplest(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn.ledField |= (1<<led);
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn.ledField &= ~(1<<led);
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        IsLEDOn.ledField |= (1<<led);
    } else {
         IsLEDOn.ledField &= ~(1<<led);
    }
}

void UpdateLEDWithDecay(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn.ledField |= (1<<led);
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn.ledField &= ~(1<<led);
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        IsLEDOn.ledField |= (1<<led);
        LEDDecayCounter[led] = LEDDecayValues[led];
    } else if (LEDDecayValues[led] > 0 && LEDDecayCounter[led] > 0) {
        LEDDecayCounter[led]--;
        IsLEDOn.ledField |= (1<<led);
    } else {
        IsLEDOn.ledField &= ~(1<<led);
    }
}

void UpdateLEDWithDecayAndMaxTime(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn.ledField |= (1<<led);
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn.ledField &= ~(1<<led);
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        if (LEDDecayValues[led] > 0) {
            LEDDecayCounter[led] = LEDDecayValues[led];
        }
        if (LEDMaxTimeOnValues[led] > 0) {
            if (LEDMaxTimeOnCounter[led]-- <= 0) {
                IsLEDOn.ledField &= ~(1<<led);
            } else {
                IsLEDOn.ledField |= (1<<led);
            }
        } else {
            IsLEDOn.ledField |= (1<<led);
        }

    } else {
        if (LEDDecayValues[led] > 0) {
            if (LEDDecayCounter[led]-- > 0) {
                if (LEDMaxTimeOnValues[led] > 0) {
                    if (LEDMaxTimeOnCounter[led]-- <= 0) {
                        IsLEDOn.ledField &= ~(1<<led);
                    } else {
                        IsLEDOn.ledField |= (1<<led);
                    }
                } else {
                    IsLEDOn.ledField |= (1<<led);
                }
            } else {
                IsLEDOn.ledField &= ~(1<<led);
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        } else {
            IsLEDOn.ledField &= ~(1<<led);
            LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
        }
    }
}

// This function is ONLY used when delay > 0.
// We ignore maxtime on for this because the lights on time
// is determined uniquely by the decay time.
// We also require a decay > 0.

void UpdateLEDWithDelay(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn.ledField |= (1<<led);
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn.ledField &= ~(1<<led);
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        IsLEDOn.ledField &= ~(1<<led);
        LEDDelayCounter[led] = LEDDelayValues[led];
        LEDDecayCounter[led] = LEDDecayValues[led];
    } else {
        if (LEDDelayCounter[led]-- > 0) {
            IsLEDOn.ledField &= ~(1<<led);
            LEDDecayCounter[led] = LEDDecayValues[led];
        } else {
            if (LEDDecayCounter[led]-- >= 0) {
                IsLEDOn.ledField |= (1<<led);
            } else {
                IsLEDOn.ledField &= ~(1<<led);
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
        LEDDelayCounter[i] = LEDDelayValues[i];
        LEDMaxTimeOnCounter[i] = LEDMaxTimeOnValues[i];
        IsLEDOn.ledField &= ~(1<<i);
    }
    if (delayval == 0 && maxtimeonval == 0 && decayval == 0) {
        LEDUpdateFunction = &UpdateLEDSimplest;
    } else if (delayval == 0 && maxtimeonval == 0) {
        LEDUpdateFunction = &UpdateLEDWithDecay;
    } else if (delayval == 0) {
        LEDUpdateFunction = &UpdateLEDWithDecayAndMaxTime;
    } else {
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


void SetCurrentOptoState() {
    int OptoState1, OptoState2;
    OptoState1 = OptoState2 = 0;
    if (IsLEDOn.bits.LED1)
        OptoState1 = 0x01;
    if (IsLEDOn.bits.LED2)
        OptoState2 = 0x01;
    if (IsLEDOn.bits.LED3)
        OptoState1 |= 0x02;
    if (IsLEDOn.bits.LED4)
        OptoState2 |= 0x02;
    if (IsLEDOn.bits.LED5)
        OptoState1 |= 0x04;
    if (IsLEDOn.bits.LED6)
        OptoState2 |= 0x04;
    if (IsLEDOn.bits.LED7)
        OptoState1 |= 0x08;
    if (IsLEDOn.bits.LED8)
        OptoState2 |= 0x08;
    if (IsLEDOn.bits.LED9)
        OptoState1 |= 0x10;
    if (IsLEDOn.bits.LED10)
        OptoState2 |= 0x10;
    if (IsLEDOn.bits.LED11)
        OptoState1 |= 0x20;
    if (IsLEDOn.bits.LED12)
        OptoState2 |= 0x20;
    SetOptoState(OptoState1, OptoState2);
}

void StepLEDControl() {
    unsigned char i;
    for (i = 0; i < 12; i++)
        LEDUpdateFunction(i); 
    SetCurrentOptoState();
}
