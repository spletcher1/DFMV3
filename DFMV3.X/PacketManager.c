#include "GlobalIncludes.h"


struct StatusPacket stausBuffer[16]; 
unsigned char inputpointer = 0, outputpointer = 0;

extern errorFlags_t volatile currentError;

extern unsigned char dfmID;
extern unsigned char isInDarkMode;
extern unsigned char volatile OptoState1;
extern unsigned char volatile OptoState2;
extern unsigned int volatile pulseWidth_ms;
extern unsigned int volatile hertz;

extern unsigned int TSL2591_LUX;
extern unsigned int Si7021_Humidity;
extern unsigned int Si7021_Temperature;

int counter100ms;

void InitializeStatusPacketBuffer(){
    counter100ms=0;
}

struct StatusPacket *GetStatusAtFront(){
    struct StatusPacket *tmp = &stausBuffer[outputpointer++];
    if(tmp->HasBeenPulled==1)
        currentError.bits.STATUSBUFFER=1;
    else
        currentError.bits.STATUSBUFFER=0;
    if(outputpointer>=16)
        outputpointer=0;
    return tmp;
}

void AddCurrentStatus() {
    unsigned int checksum=0,i;    
    unsigned char *statusPointer;
    stausBuffer[inputpointer].Header1=0xFF;
    stausBuffer[inputpointer].Header2=0xFF;
    stausBuffer[inputpointer].Header3=0xFD;   
    stausBuffer[inputpointer].ID=dfmID;     
    stausBuffer[inputpointer].ErrorFlag = currentError.byte;
    statusPointer = (char *)&stausBuffer[inputpointer].Header1;  
    FillCurrentStatus(&stausBuffer[inputpointer]);
    stausBuffer[inputpointer].Optostate1 = OptoState1;
    stausBuffer[inputpointer].Optostate2 = OptoState2;
    stausBuffer[inputpointer].OptoFreq1 = hertz >> 8;
    stausBuffer[inputpointer].OptoFreq2 = hertz & 0xFF;
    stausBuffer[inputpointer].OptoPW1 = pulseWidth_ms >> 8;
    stausBuffer[inputpointer].OptoPW2 = pulseWidth_ms & 0xFF;
    stausBuffer[inputpointer].DarkMode = isInDarkMode;
    stausBuffer[inputpointer].Temperature1 = (Si7021_Temperature >> 24);
    stausBuffer[inputpointer].Temperature2 = ((Si7021_Temperature >> 16) & 0xFF);
    stausBuffer[inputpointer].Temperature3 = ((Si7021_Temperature >> 8) & 0xFF);
    stausBuffer[inputpointer].Temperature4 = (Si7021_Temperature & 0xFF);
    stausBuffer[inputpointer].Humidity1 = (Si7021_Humidity >> 24);
    stausBuffer[inputpointer].Humidity2 = ((Si7021_Humidity >> 16) & 0xFF);
    stausBuffer[inputpointer].Humidity3 = ((Si7021_Humidity >> 8) & 0xFF);
    stausBuffer[inputpointer].Humidity4 = (Si7021_Humidity & 0xFF);
    stausBuffer[inputpointer].LightHigh = TSL2591_LUX >> 8;
    stausBuffer[inputpointer].LightLow = TSL2591_LUX & 0xFF;
    for (i = 3; i < (STATUSPACKETSIZE - 4); i++) checksum += *(statusPointer + i);
    checksum = (checksum ^ 0xFFFFFFFF) + 0x01;
    stausBuffer[inputpointer].Checksum1 = checksum >> 24;
    stausBuffer[inputpointer].Checksum2 = (checksum >> 16) & 0xFF;
    stausBuffer[inputpointer].Checksum3 = (checksum >> 8) & 0xFF;
    stausBuffer[inputpointer].Checksum4 = checksum & 0xFF;
    if(++inputpointer>=16)
        inputpointer=0;    
    currentError.byte = 0x00;
}

// This function is meant to be called every 100ms
void StepPacketManager(){
    if(++counter100ms>=2){
        AddCurrentStatus();
        counter100ms=0;
    }    
}