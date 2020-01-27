
int LEDDecayCounter[12];
int LEDDelayCounter[12];
int LEDMaxTimeOnCounter[12];

unsigned int LEDDecayValues[12];
unsigned int LEDDelayValues[12];
unsigned int LEDMaxTimeOnValues[12];

unsigned char IsLEDOn[12];
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
        IsLEDOn[led] = 1;
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn[led] = 0;
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        IsLEDOn[led] = 1;
    } else {
        IsLEDOn[led] = 0;
    }
}

void UpdateLEDWithDecay(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn[led] = 1;
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn[led] = 0;
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        IsLEDOn[led] = 1;
        LEDDecayCounter[led] = LEDDecayValues[led];
    } else if (LEDDecayValues[led] > 0 && LEDDecayCounter[led] > 0) {
        LEDDecayCounter[led]--;
        IsLEDOn[led] = 1;
    } else {
        IsLEDOn[led] = 0;
    }
}

void UpdateLEDWithDecayAndMaxTime(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn[led] = 1;
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn[led] = 0;
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        if (LEDDecayValues[led] > 0) {
            LEDDecayCounter[led] = LEDDecayValues[led];
        }
        if (LEDMaxTimeOnValues[led] > 0) {
            if (LEDMaxTimeOnCounter[led]-- <= 0) {
                IsLEDOn[led] = 0;
            } else {
                IsLEDOn[led] = 1;
            }
        } else {
            IsLEDOn[led] = 1;
        }

    } else {
        if (LEDDecayValues[led] > 0) {
            if (LEDDecayCounter[led]-- > 0) {
                if (LEDMaxTimeOnValues[led] > 0) {
                    if (LEDMaxTimeOnCounter[led]-- <= 0) {
                        IsLEDOn[led] = 0;
                    } else {
                        IsLEDOn[led] = 1;
                    }
                } else {
                    IsLEDOn[led] = 1;
                }
            } else {
                IsLEDOn[led] = 0;
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        } else {
            IsLEDOn[led] = 0;
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
        IsLEDOn[led] = 1;
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn[led] = 0;
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        IsLEDOn[led] = 0;
        LEDDelayCounter[led] = LEDDelayValues[led];
        LEDDecayCounter[led] = LEDDecayValues[led];
    } else {
        if (LEDDelayCounter[led]-- > 0) {
            IsLEDOn[led] = 0;
            LEDDecayCounter[led] = LEDDecayValues[led];
        } else {
            if (LEDDecayCounter[led]-- >= 0) {
                IsLEDOn[led] = 1;
            } else {
                IsLEDOn[led] = 0;
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            }
        }
    }
}


// This function was deprecated because it is too complicated
// and not required.
// This is because when there is a delay, the lights on time
// should be determined specifically by the decay
void UpdateLEDFullOLD(unsigned char led) {
    if (LEDThresholdValues[led] == 0) {
        IsLEDOn[led] = 1;
        return;
    }
    if (LEDThresholdValues[led] == -1) {
        IsLEDOn[led] = 0;
        return;
    }
    if (CurrentValues[led] > LEDThresholdValues[led]) {
        if (LEDDelayValues[led] > 0) {
            IsLEDOn[led] = 0;
            LEDDelayCounter[led] = LEDDelayValues[led];
            LEDDecayCounter[led] = LEDDecayValues[led];
            LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
        } else {
            if (LEDDecayValues[led] > 0) {
                LEDDecayCounter[led] = LEDDecayValues[led];
            }
            if (LEDMaxTimeOnValues[led] > 0) {
                if (LEDMaxTimeOnCounter[led]-- <= 0) {
                    IsLEDOn[led] = 0;
                } else {
                    IsLEDOn[led] = 1;
                }
            }
            else {
                IsLEDOn[led] = 1;
            }
        }
    } else {
        if (LEDDelayValues[led] > 0) {
            if (LEDDelayCounter[led]-- > 0) {
                IsLEDOn[led] = 0;
                LEDDecayCounter[led] = LEDDecayValues[led];
                LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
            } else {
                if (LEDDecayCounter[led]-- >= 0) {
                    if (LEDMaxTimeOnValues[led] > 0) {
                        if (LEDMaxTimeOnCounter[led]-- <= 0) {
                            IsLEDOn[led] = 0;
                        } else {
                            IsLEDOn[led] = 1;
                        }
                    } else {
                        IsLEDOn[led] = 1;
                    }
                } else {
                    IsLEDOn[led] = 0;
                    LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
                }
            }
        } else {
            if (LEDDecayValues[led] > 0) {
                if (LEDDecayCounter[led]-- > 0) {
                    if (LEDMaxTimeOnValues[led] > 0) {
                        if (LEDMaxTimeOnCounter[led]-- <= 0) {
                            IsLEDOn[led] = 0;
                        } else {
                            IsLEDOn[led] = 1;
                        }
                    } else {
                        IsLEDOn[led] = 1;
                    }
                } else {
                    IsLEDOn[led] = 0;
                    LEDMaxTimeOnCounter[led] = LEDMaxTimeOnValues[led];
                }
            } else {
                IsLEDOn[led] = 0;
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
        IsLEDOn[i] = 0;
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


void SetCurrentOptoStateOld() {
    int OptoState1, OptoState2;
    OptoState1 = OptoState2 = 0;
    if (IsLEDOn[0])
        OptoState1 = 0x01;
    if (IsLEDOn[1])
        OptoState2 = 0x01;
    if (IsLEDOn[2])
        OptoState1 |= 0x02;
    if (IsLEDOn[3])
        OptoState2 |= 0x02;
    if (IsLEDOn[4])
        OptoState1 |= 0x04;
    if (IsLEDOn[5])
        OptoState2 |= 0x04;
    if (IsLEDOn[6])
        OptoState1 |= 0x08;
    if (IsLEDOn[7])
        OptoState2 |= 0x08;
    if (IsLEDOn[8])
        OptoState1 |= 0x10;
    if (IsLEDOn[9])
        OptoState2 |= 0x10;
    if (IsLEDOn[10])
        OptoState1 |= 0x20;
    if (IsLEDOn[11])
        OptoState2 |= 0x20;
    SetOptoState(OptoState1, OptoState2);
}
//For camera control
void SetCurrentOptoState(){
    int i,OptoState1, OptoState2,sum=0;
    OptoState1 = OptoState2 = 0;
    for(i=0;i<12;i++)
        sum+=IsLEDOn[i];
    if(sum>0){
        OptoState1=OptoState2=0x3F;        
    }
    SetOptoState(OptoState1, OptoState2);            
}
void StepLEDControl() {
    unsigned char i;
    for (i = 0; i < 12; i++)
        LEDUpdateFunction(i);
    SetCurrentOptoState();
}
