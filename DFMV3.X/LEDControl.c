
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

void SetLEDThresholds(int *thresh){
    int i;
    for(i=0;i<12;i++)
        LEDThresholdValues[i]=thresh[i];
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

void UpdateLEDSecondSimplest(unsigned char led) {
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

void UpdateLEDThirdSimplest(unsigned char led) {
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

void UpdateLEDFull(unsigned char led) {
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


void SetLEDParams(unsigned int decayval, unsigned int delayval, unsigned int maxtimeonval) {
    int i;
    for (i = 0; i < 12; i++) {
        LEDDecayValues[i] = decayval;
        LEDDelayValues[i] = delayval;
        LEDMaxTimeOnValues[i] = maxtimeonval;

        LEDDecayCounter[i] = 0;
        LEDDelayCounter[i] = LEDDelayValues[i];
        LEDMaxTimeOnCounter[i] = LEDMaxTimeOnCounter[i];
        IsLEDOn[i] = 0;
    }
    if (delayval == 0 && maxtimeonval == 0 && decayval == 0) {
        LEDUpdateFunction = &UpdateLEDSimplest;
    } else if (delayval == 0 && maxtimeonval == 0) {
        LEDUpdateFunction = &UpdateLEDSecondSimplest;
    } else if (delayval == 0) {
        LEDUpdateFunction = &UpdateLEDThirdSimplest;
    } else {
        LEDUpdateFunction = &UpdateLEDFull;
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

void StepLEDControl() {
    unsigned char i;
    for (i = 0; i < 12; i++)
        LEDUpdateFunction(i);
    SetCurrentOptoState();
}
