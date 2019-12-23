
int LEDDecayCounter[12];
int LEDDelayCounter[12];
int LEDMaxTimeOnCounter[12];

unsigned int LEDDecayValues[12];
unsigned int LEDDelayValues[12];
unsigned int LEDMaxTimeOnValues[12];

unsigned char IsLEDOn[12];
unsigned char IsOverMaxTimeOn[12]; 
unsigned char isLastSignalOverThreshold;

extern int CurrentValues[13]; // This has one extra place for the input voltage reading from the DFM.
int LEDThresholdValues[12];

void InitializeLEDControl(){
    int i;
    // Contstant off for now
    // Delay=Decay=0
    // MaxTimeOn=0
    for (i=0;i<12;i++){
        LEDThresholdValues[i]=100*128;
        LEDDecayValues[i]=1000;
        LEDDelayValues[i]=0;
        LEDMaxTimeOnValues[i]=2000;
        
        LEDDecayCounter[i]=LEDDecayValues[i];
        LEDDelayCounter[i]=LEDDelayValues[i];
        LEDMaxTimeOnCounter[i]=LEDMaxTimeOnCounter[i];                
        IsLEDOn[i]=0;
    }    
    
}

void UpdateLEDPartial(int led){
    if(LEDThresholdValues[led]==0){
        IsLEDOn[led]=1;
        return;
    }
    if(LEDThresholdValues[led]==-1){
        IsLEDOn[led]=0;
        return;                
    }
    if(CurrentValues[led]>LEDThresholdValues[led]){
        IsLEDOn[led]=1;
        LEDDecayCounter[led]=LEDDecayValues[led];
    }
    else if(LEDDecayValues[led]>0 && LEDDecayCounter[led]>0){
        LEDDecayCounter[led]--;
        IsLEDOn[led]=1;
    }
    else{
        IsLEDOn[led]=0;       
    }    
}

void UpdateLED(int led){
    if(LEDThresholdValues[led]==0){
        IsLEDOn[led]=1;
        return;
    }
    if(LEDThresholdValues[led]==-1){
        IsLEDOn[led]=0;
        return;                
    }
    if(CurrentValues[led]>LEDThresholdValues[led]){
        if(LEDDelayValues[led]>0){
            IsLEDOn[led]=0;
            LEDDelayCounter[led]=LEDDelayValues[led];
            LEDDecayCounter[led]=LEDDecayValues[led];    
            LEDMaxTimeOnCounter[led]=LEDMaxTimeOnValues[led];
        }
        else{
            if(LEDDecayValues[led]>0){
                LEDDecayCounter[led]=LEDDecayValues[led];
            }
            if(LEDMaxTimeOnValues[led] > 0){
                if(LEDMaxTimeOnCounter[led]-- <=0){
                    IsLEDOn[led]=0;
                }
                else {
                    IsLEDOn[led]=1;                    
                }
            }   
            else {
                IsLEDOn[led]=1;
            }
        }        
    }
    else {
        if(LEDDelayValues[led]>0){
            if(LEDDelayCounter[led]-->0){
                IsLEDOn[led]=0;
                LEDDecayCounter[led]=LEDDecayValues[led];
                LEDMaxTimeOnCounter[led]=LEDMaxTimeOnValues[led];                                           
            }
            else{
                if(LEDDecayCounter[led]-->=0){                   
                    if(LEDMaxTimeOnValues[led] > 0){
                        if(LEDMaxTimeOnCounter[led]-- <=0){
                            IsLEDOn[led]=0;                            
                        }
                        else {
                            IsLEDOn[led]=1;
                        }
                    }
                    else {
                        IsLEDOn[led]=1;
                    }
                }
                else {
                    IsLEDOn[led]=0;
                    LEDMaxTimeOnCounter[led]=LEDMaxTimeOnValues[led];
                }
            }        
        }
        else {
            if(LEDDecayValues[led]>0){
                if(LEDDecayCounter[led]-->0){
                    if(LEDMaxTimeOnValues[led] > 0){
                        if(LEDMaxTimeOnCounter[led]-- <=0){
                            IsLEDOn[led]=0;                            
                        }
                        else {
                            IsLEDOn[led]=1;
                        }
                    }
                    else {
                        IsLEDOn[led]=1;
                    }
                }
                else {
                    IsLEDOn[led]=0;
                    LEDMaxTimeOnCounter[led]=LEDMaxTimeOnValues[led];
                }
            }
            else {
                IsLEDOn[led]=0;
                LEDMaxTimeOnCounter[led]=LEDMaxTimeOnValues[led];
            }
        }
    }
}

void SetCurrentOptoState(){    
    int OptoState1,OptoState2;
    OptoState1=OptoState2=0;    
    if(IsLEDOn[0])
        OptoState1=0x01;
    if(IsLEDOn[1])
        OptoState2=0x01;
    if(IsLEDOn[2])
        OptoState1|=0x02;
    if(IsLEDOn[3])
        OptoState2|=0x02;
    if(IsLEDOn[4])
        OptoState1|=0x04;
    if(IsLEDOn[5])
        OptoState2|=0x04;
    if(IsLEDOn[6])
        OptoState1|=0x08;
    if(IsLEDOn[7])
        OptoState2|=0x08;
    if(IsLEDOn[8])
        OptoState1|=0x10;
    if(IsLEDOn[9])
        OptoState2|=0x10;
    if(IsLEDOn[10])
        OptoState1|=0x20;
    if(IsLEDOn[11])
        OptoState2|=0x20;    
    SetOptoState(OptoState1,OptoState2);  
}

void StepLEDControl(){
    int i;
    for(i=0;i<12;i++)
        UpdateLED(i);
    SetCurrentOptoState();
}
