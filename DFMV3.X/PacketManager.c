#include "GlobalIncludes.h"


struct StatusPacket statusBuffer[16]; 
unsigned char inputpointer = 0, outputpointer = 0;
unsigned char appendedStatusPackets[309];

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
    int i;
    counter100ms=0;
    // We set this to avoid error on first use.
    for(i=0;i<16;i++){
        statusBuffer[i].HasBeenPulled=1;
    }
    
}

struct StatusPacket *GetNextStatusInLine(){
    struct StatusPacket *tmp = &statusBuffer[outputpointer++];
    if(tmp->HasBeenPulled==1)
        tmp->ErrorFlag = tmp->ErrorFlag | 0x40;        
  
    tmp->HasBeenPulled = 1;
  
    if(outputpointer>=16)
        outputpointer=0;
    return tmp;
}


void AddCurrentStatus() {
    unsigned int checksum=0,i;    
    unsigned char *statusPointer;
    statusBuffer[inputpointer].Header1=0xFF;
    statusBuffer[inputpointer].Header2=0xFF;
    statusBuffer[inputpointer].Header3=0xFD;   
    statusBuffer[inputpointer].ID=dfmID;     
    statusBuffer[inputpointer].ErrorFlag = currentError.byte;
    statusPointer = (char *)&statusBuffer[inputpointer].Header1;  
    FillCurrentStatus(&statusBuffer[inputpointer]);
    statusBuffer[inputpointer].Optostate1 = OptoState1;
    statusBuffer[inputpointer].Optostate2 = OptoState2;
    statusBuffer[inputpointer].OptoFreq1 = hertz >> 8;
    statusBuffer[inputpointer].OptoFreq2 = hertz & 0xFF;
    statusBuffer[inputpointer].OptoPW1 = pulseWidth_ms >> 8;
    statusBuffer[inputpointer].OptoPW2 = pulseWidth_ms & 0xFF;
    statusBuffer[inputpointer].DarkMode = isInDarkMode;
    statusBuffer[inputpointer].Temperature1 = (Si7021_Temperature >> 24);
    statusBuffer[inputpointer].Temperature2 = ((Si7021_Temperature >> 16) & 0xFF);
    statusBuffer[inputpointer].Temperature3 = ((Si7021_Temperature >> 8) & 0xFF);
    statusBuffer[inputpointer].Temperature4 = (Si7021_Temperature & 0xFF);
    statusBuffer[inputpointer].Humidity1 = (Si7021_Humidity >> 24);
    statusBuffer[inputpointer].Humidity2 = ((Si7021_Humidity >> 16) & 0xFF);
    statusBuffer[inputpointer].Humidity3 = ((Si7021_Humidity >> 8) & 0xFF);
    statusBuffer[inputpointer].Humidity4 = (Si7021_Humidity & 0xFF);
    statusBuffer[inputpointer].LightHigh = TSL2591_LUX >> 8;
    statusBuffer[inputpointer].LightLow = TSL2591_LUX & 0xFF;   
    
    for (i = 4; i < (STATUSPACKETSIZE - 4); i++) checksum += *(statusPointer + i);
    checksum = (checksum ^ 0xFFFFFFFF) + 0x01;
    statusBuffer[inputpointer].Checksum1 = checksum >> 24;
    statusBuffer[inputpointer].Checksum2 = (checksum >> 16) & 0xFF;
    statusBuffer[inputpointer].Checksum3 = (checksum >> 8) & 0xFF;
    statusBuffer[inputpointer].Checksum4 = checksum & 0xFF;
           
    statusBuffer[inputpointer].HasBeenPulled = 0;
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