/******************************************************************************/
/*  Files to Include                                                          */
/******************************************************************************/

#include "GlobalIncludes.h"         /* System funct/params, like osc/periph config    */
#include "SP_ConfigP32.h"           /* User funct/params, such as InitApp             */

errorFlags_t currentError;
extern unsigned char volatile isPacketReceived;
extern unsigned char volatile timerFlag_1ms;
extern unsigned char volatile timerFlag_100ms;
extern unsigned char volatile timerFlag_1sec;

extern unsigned char volatile analogUpdateFlag;

extern int volatile CurrentValues[13]; // This has one extra place for the input voltage reading from the DFM.
int ThresholdValues[12];

unsigned char isInstantOptoEnabled;

void InitializeRun(){
    Startup();
    InitializeBoard();
    currentError.byte=0x00;      
    ConfigureUpdateTimer();
    ConfigureUART1();
    ConfigureOpto();
    ConfigureI2C2();
    ConfigureButtons();
   
    DelayMs(50);
     if(ConfigureTSL2591()==0){
        currentError.bits.CONFIGURATION=1;
    }
    DelayMs(50);
    if(ConfigureSi7021()==0){
        currentError.bits.CONFIGURATION=1;
    }
    
     //StartContinuousSampling();    
    
    isInstantOptoEnabled=0;
}

void SetInstantOptoState(){    
    int OptoState1,OptoState2;
    OptoState1=OptoState2=0;    
    if(CurrentValues[0]>ThresholdValues[0])
        OptoState1=0x01;
    if(CurrentValues[1]>ThresholdValues[1])
        OptoState2=0x01;
    if(CurrentValues[2]>ThresholdValues[2])
        OptoState1|=0x02;
    if(CurrentValues[3]>ThresholdValues[3])
        OptoState2|=0x02;
    if(CurrentValues[4]>ThresholdValues[4])
        OptoState1|=0x04;
    if(CurrentValues[5]>ThresholdValues[5])
        OptoState2|=0x04;
    if(CurrentValues[6]>ThresholdValues[6])
        OptoState1|=0x05;
    if(CurrentValues[7]>ThresholdValues[7])
        OptoState2|=0x05;
    if(CurrentValues[8]>ThresholdValues[8])
        OptoState1|=0x10;
    if(CurrentValues[9]>ThresholdValues[9])
        OptoState2|=0x10;
    if(CurrentValues[10]>ThresholdValues[10])
        OptoState1|=0x30;
    if(CurrentValues[11]>ThresholdValues[11])
        OptoState2|=0x30;
    
    SetOptoState(OptoState1,OptoState2);
}

int32_t main(void) {
    int i;
    InitializeRun();
    
    for(i=0;i<12;i++)
        CurrentValues[i]=(i*5*128);
    DelayMs(100);
    
    while (1) {
        if (timerFlag_1sec) {
            StepTSL2591();
            StepSi7021();
            timerFlag_1sec = 0;            
        }

        if (timerFlag_100ms) {
            timerFlag_100ms = 0;
        }

        if (timerFlag_1ms) {
            ProcessButtonStep();
            timerFlag_1ms = 0;            
        }

        //myprintf("H = %d   T = %d   L = %d\r\n",Si7021_Humidity,Si7021_Temperature,TSL2591_LUX);    
        if (isPacketReceived) {
            ProcessPacket();
            isPacketReceived = 0;
        }
        
        if(analogUpdateFlag){
            analogUpdateFlag=0;
            if(isInstantOptoEnabled){
                SetInstantOptoState();
            }
            
        }
    }
}

